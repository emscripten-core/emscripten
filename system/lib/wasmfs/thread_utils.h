/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <functional>
#include <thread>

#include <emscripten/promise.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

extern "C" {
void _wasmfs_thread_utils_heartbeat(em_proxying_queue* ctx);
}

namespace emscripten {

// Helper class for synchronously proxying work to a dedicated worker thread,
// including where the work is asynchronous.
class ProxyWorker {
public:
  enum Synchronicity { Sync, Async };

private:
  // The queue we use to proxy work.
  ProxyingQueue queue;

  // Determines how we notify the caller that the worker is ready to receive
  // work.
  Synchronicity synchronicity;

  // The thread constructing this ProxyWorker, notified when we are starting
  // asynchronously.
  pthread_t caller;

  // When we are starting asynchronously, this promise will be resolved once we
  // are ready to receive work.
  em_promise_t startedPromise;

  // The dedicated worker thread.
  std::thread thread;

  // Used to notify the calling thread once the worker has been started when we
  // are starting synchronously.
  bool started = false;
  std::mutex mutex;
  std::condition_variable cond;

public:
  // enum Synchronicity { Sync, Async };

  // Spawn the worker thread.
  ProxyWorker(Synchronicity sync)
    : queue(), synchronicity(sync), caller(pthread_self()),
      startedPromise(synchronicity == Async ? emscripten_promise_create()
                                            : nullptr),
      thread([&]() {
        // Notify the caller that we have started.
        if (synchronicity == Async) {
          // Resolve the promise on the caller thread.
          queue.proxyAsync(caller, [&] {
            emscripten_promise_resolve(
              startedPromise, EM_PROMISE_FULFILL, nullptr);
          });
        } else {
          // Notify the condition variable.
          {
            std::unique_lock<std::mutex> lock(mutex);
            started = true;
          }
          cond.notify_all();
        }

        // Sometimes the main thread is spinning, waiting on a WasmFS
        // lock held by a thread trying to proxy work to this dedicated
        // worker. In that case, the proxying message won't be relayed
        // by the main thread and the system will deadlock. This
        // heartbeat ensures that proxying work eventually gets done so
        // the thread holding the lock can make forward progress even
        // if the main thread is blocked.
        //
        // TODO: Remove this once we can postMessage directly between
        // workers without involving the main thread or once all
        // browsers ship Atomics.waitAsync.
        //
        // Note that this requires adding
        // _emscripten_proxy_execute_queue to EXPORTED_FUNCTIONS.
        _wasmfs_thread_utils_heartbeat(queue.queue);

        // Sit in the event loop performing work as it comes in.
        emscripten_exit_with_live_runtime();
      }) {

    // Make sure the thread has actually started before returning. This allows
    // subsequent code to assume the thread has already been spawned and not
    // worry about potential deadlocks where it holds a lock while proxying an
    // operation and meanwhile the main thread is blocked trying to acquire the
    // same lock so is never able to spawn the worker thread.
    //
    // Unfortunately, this solution would cause the main thread to deadlock on
    // itself, so for now assert that we are not on the main thread.
    if (synchronicity == Sync) {
      assert(!emscripten_is_main_browser_thread() &&
             "cannot safely spawn dedicated workers from "
             "the main browser thread");
      {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [&]() { return started; });
      }
    }
  }

  // Kill the worker thread.
  ~ProxyWorker() {
    pthread_cancel(thread.native_handle());
    thread.join();
    if (synchronicity == Async) {
      emscripten_promise_destroy(startedPromise);
    }
  }

  // Proxy synchronous work.
  void operator()(const std::function<void()>& func) {
    queue.proxySync(thread.native_handle(), func);
  }
  // Proxy asynchronous work that calls `finish()` on the ctx parameter to mark
  // its end.
  void operator()(const std::function<void(ProxyingQueue::ProxyingCtx)>& func) {
    queue.proxySyncWithCtx(thread.native_handle(), func);
  }

  // If the worker is starting asynchronously, get the promise that will be
  // resolved when it is done starting up. The promise handle is owned by and is
  // valid for the lifetime of the ProxyWorker.
  em_promise_t getPromise() {
    assert(synchronicity == Async);
    return startedPromise;
  }
};

} // namespace emscripten
