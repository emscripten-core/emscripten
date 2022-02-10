/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <assert.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <pthread.h>

#include <functional>
#include <thread>
#include <utility>

// TODO: This will be updated with:
// https://github.com/emscripten-core/emscripten/pull/15681

namespace emscripten {

// Helper class for generic sync-to-async conversion. Creating an instance of
// this class will spin up a pthread. You can then call invoke() to run code
// on that pthread. The work done on the pthread receives a callback method
// which lets you indicate when it finished working. The call to invoke() is
// synchronous, while the work done on the other thread can be asynchronous,
// which allows bridging async JS APIs to sync C++ code.
//
// This can be useful if you are in a location where blocking is possible (like
// a thread, or when using PROXY_TO_PTHREAD), but you have code that is hard to
// refactor to be async, but that requires some async operation (like waiting
// for a JS event).
class SyncToAsync {

  // Public API
  //==============================================================================
public:
  // Pass around the callback as a pointer to a std::function. Using a pointer
  // means that it can be sent easily to JS, as a void* parameter to a C API,
  // etc., and also means we do not need to worry about the lifetime of the
  // std::function in user code.
  using Callback = std::function<void()>*;

  //
  // Run some work on thread. This is a synchronous (blocking) call. The thread
  // where the work actually runs can do async work for us - all it needs to do
  // is call the given callback function when it is done.
  //
  // Note that you need to call the callback even if you are not async, as the
  // code here does not know if you are async or not. For example,
  //
  //  instance.invoke([](emscripten::SyncToAsync::Callback resume) {
  //    std::cout << "Hello from sync C++ on the pthread\n";
  //    (*resume)();
  //  });
  //
  // In the async case, you would call resume() at some later time.
  //
  // It is safe to call this method from multiple threads, as it locks itself.
  // That is, you can create an instance of this and call it from multiple
  // threads freely.
  //
  void invoke(std::function<void(Callback)> newWork);

  //==============================================================================
  // End Public API

private:
  // The dedicated worker thread.
  std::thread thread;

  // Condition variable used for bidirectional communication between the worker
  // thread and invoking threads.
  std::condition_variable condition;
  std::mutex mutex;

  // The current state of the worker thread. New work can only be submitted when
  // in the `Waiting` state.
  enum State {
    Waiting,
    WorkAvailable,
    ShouldExit,
  } state = Waiting;

  // Increment the count every time work is finished. This will allow invokers
  // to detect that their particular work has been completed even if some other
  // invoker wins the race and submits new work before the original invoker can
  // check for completion.
  std::atomic<uint32_t> workCount{0};

  // The work that the dedicated worker thread should perform and the callback
  // that needs to be called when the work is finished.
  std::function<void(Callback)> work;
  std::function<void()> resume;

  static void* threadMain(void* arg) {
    // Schedule ourselves to start processing incoming work requests.
    emscripten_async_call(threadIter, arg, 0);
    return 0;
  }

  // The main worker thread routine that waits for work, wakes up when work is
  // available, executes the work, then schedules itself again.
  static void threadIter(void* arg) {
    auto* parent = (SyncToAsync*)arg;

    std::function<void(Callback)> currentWork;
    {
      // Wait until we get something to do.
      std::unique_lock<std::mutex> lock(parent->mutex);
      parent->condition.wait(lock, [&]() {
        return parent->state == WorkAvailable || parent->state == ShouldExit;
      });

      if (parent->state == ShouldExit) {
        pthread_exit(0);
      }

      assert(parent->state == WorkAvailable);
      currentWork = parent->work;

      // Now that we have a local copy of the work, it is ok for new invokers to
      // queue up more work for us to do, so go back to `Waiting` and release
      // the lock.
      parent->state = Waiting;
    }

    // Allocate a resume function that will wake the invoker and schedule us to
    // wait for more work.
    parent->resume = [parent, arg]() {
      // We are called, so the work was finished. Notify the invoker so it will
      // wake up and continue once we resume waiting. There might be other
      // invokers waiting to give us work, so `notify_all` to make sure our
      // invoker wakes up. Don't worry about overflow because it's a reasonable
      // assumption that no invoker will continue losing wake up races for a
      // full cycle.
      parent->workCount++;
      parent->condition.notify_all();

      // Look for more work. Doing this asynchronously ensures that we continue
      // after the current call stack unwinds (avoiding constantly adding to the
      // stack, and also running any remaining code the caller had, like
      // destructors). TODO: add an option to do a synchronous call here in some
      // cases, which would avoid the time delay caused by a browser setTimeout.
      emscripten_async_call(threadIter, arg, 0);
    };

    // Run the work function the user gave us. Give it a pointer to the resume
    // function, which it will be responsible for calling when it's done.
    currentWork(&parent->resume);
  }

public:
  // Spawn the worker thread. It starts in the `Waiting` state, so it is ready
  // to accept work requests from invokers even before it starts up.
  SyncToAsync() : thread(threadMain, this) {}

  ~SyncToAsync() {
    std::unique_lock<std::mutex> lock(mutex);

    // We are destructing the SyncToAsync object, so we should not be racing
    // with other threads trying to perform more `invoke`s. There should
    // therefore not be any work available.
    assert(state == Waiting);

    // Wake the worker and tell it to quit. Be ready to join it when it does.
    // There shouldn't be other invokers waiting to send work since we are
    // destructing the SyncToAsync, so just use `notify_one`.
    state = ShouldExit;
    condition.notify_one();

    // Unlock to allow the worker to wake up and exit.
    lock.unlock();
    thread.join();
  }
};

inline void SyncToAsync::invoke(std::function<void(Callback)> newWork) {
  // The worker might not be waiting for work if some other invoker has already
  // sent work. Wait for the worker to be done with that work and ready for new
  // work.
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock, [&]() { return state == Waiting; });

  // Now the worker is definitely waiting for our work. Send it over.
  assert(state == Waiting);
  uint32_t workID = workCount;
  work = newWork;
  state = WorkAvailable;

  // Wake the worker and wait for it to finish the work. There might be other
  // invokers waiting to send work as well, so `notify_all` to ensure the worker
  // wakes up. Wait for `workCount` to increase rather than for the state to
  // return to `Waiting` to make sure we wake up even if some other invoker wins
  // the race and submits more work before we acquire the lock.
  condition.notify_all();
  condition.wait(lock, [&]() { return workCount != workID; });
}

} // namespace emscripten
