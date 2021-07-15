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

namespace emscripten {

// Helper function to call a callable object on the main thread in a fully
// asynchronous manner.
//
// Normal proxying to the main thread runs into the possible issue of the main
// thread pumping the event queue while it blocks. That is necessary for e.g.
// WebGL proxying where both sync and async events must be received and run in
// order. With this class, only async events are allowed, and they are run in
// an async manner on the main thread, that is, with nothing else on the stack
// while they run, which avoids any possibility of another even running while
// this one is (if this one blocks on a mutex, for example, which would make the
// main thread pump the event queue as it waits).

template <class F>
void invoke_on_main_thread_async(F&& f) {
  using function_type = typename std::remove_reference<F>::type;

  auto run_on_main_thread = [](void* f1) {
    // Once on the main thread, run as an event from the JS event queue
    // directly, so nothing else is on the stack when we execute. This means
    // we are no longer ordered with respect to synchronous proxied calls,
    // but that is ok in this case as well we care about are async ones.
    emscripten_async_call(
      [](void* f2) {
        auto f = static_cast<function_type*>(f2);
        (*f)();
        delete f;
      },
      f1,
      0
    );
  };

  // Proxy the call to the main thread.
  emscripten_async_run_in_main_runtime_thread(
    EM_FUNC_SIG_VI,
    static_cast<void (*)(void*)>(run_on_main_thread),
    // If we were passed in something we need to copy, copy it; we end up with
    // a new allocation here on the heap that we will free later.
    new function_type(std::forward<F>(f))
  );
}

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
class sync_to_async {
// Public API
public:

  using Callback = std::function<void()>;

  // Run some work on thread. This is a synchronous (blocking) call. The thread
  // where the work actually runs can do async work for us - all it needs to do
  // is call the given callback function when it is done.
  void invoke(std::function<void(Callback)> newWork);

// Private API
private:
  std::thread thread;
  std::mutex mutex;
  std::condition_variable condition;
  std::function<void(Callback)> work;
  bool readyToWork = false;
  bool finishedWork;
  bool quit = false;

  // The child will be asynchronous, and therefore we cannot rely on RAII to
  // unlock for us, we must do it manually.
  std::unique_lock<std::mutex> childLock;

  static void* threadMain(void* arg) {
    auto* parent = (sync_to_async*)arg;
    emscripten_async_call(threadIter, arg, 0);
    return 0;
  }

  static void threadIter(void* arg) {
    auto* parent = (sync_to_async*)arg;
    // Wait until we get something to do.
    parent->childLock.lock();
    parent->condition.wait(parent->childLock, [&]() {
      return parent->readyToWork;
    });
    auto work = parent->work;
    parent->readyToWork = false;
    // Do the work.
    work([parent, arg]() {
      // We are called, so the work was finished. Notify the caller.
      parent->finishedWork = true;
      parent->childLock.unlock();
      parent->condition.notify_one();
      if (parent->quit) {
        // This works fine although it is a c++11 std::thread.
        pthread_exit(0);
      } else {
        // Look for more work. (We do this asynchronously to avoid nesting of
        // the stack, and to keep this function simple without a loop.)
        emscripten_async_call(threadIter, arg, 0);
      }
    });
  }

public:
  sync_to_async() : thread(threadMain, this), childLock(mutex) {
    // The child lock is associated with the mutex, which takes the lock, and
    // we free it here. Only the child will lock/unlock it from now on.
    childLock.unlock();
  }

  ~sync_to_async() {
    quit = true;

    // Wake up the child with an empty task.
    invoke([](Callback func){
      func();
    });

    thread.join();
  }
};

void sync_to_async::invoke(std::function<void(Callback)> newWork) {
  // Send the work over.
  {
    std::lock_guard<std::mutex> lock(mutex);
    work = newWork;
    finishedWork = false;
    readyToWork = true;
  }
  condition.notify_one();

  // Wait for it to be complete.
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock, [&]() {
    return finishedWork;
  });
}

} // namespace emscripten
