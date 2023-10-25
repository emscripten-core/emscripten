/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <functional>
#include <thread>

#include <emscripten/proxying.h>
#include <emscripten/threading.h>

extern "C" {
void _wasmfs_thread_utils_heartbeat(em_proxying_queue* ctx);
}

namespace emscripten {

// Helper class for synchronously proxying work to a dedicated worker thread,
// including where the work is asynchronous.
class ProxyWorker {
  // The queue we use to proxy work and the dedicated worker.
  ProxyingQueue queue;
  std::thread thread;

  // Used to notify the calling thread once the worker has been started.
  bool started = false;
  std::mutex mutex;
  std::condition_variable cond;

public:
  // Spawn the worker thread.
  ProxyWorker()
    : queue(), thread([&]() {
        // Notify the caller that we have started.
        {
          std::unique_lock<std::mutex> lock(mutex);
          started = true;
        }
        cond.notify_all();

        // Sometimes the main thread is spinning, waiting on a WasmFS lock held
        // by a thread trying to proxy work to this dedicated worker. In that
        // case, the proxying message won't be relayed by the main thread and
        // the system will deadlock. This heartbeat ensures that proxying work
        // eventually gets done so the thread holding the lock can make forward
        // progress even if the main thread is blocked.
        //
        // TODO: Remove this once we can postMessage directly between workers
        // without involving the main thread or once all browsers ship
        // Atomics.waitAsync.
        //
        // Note that this requires adding _emscripten_proxy_execute_queue to
        // EXPORTED_FUNCTIONS.
        _wasmfs_thread_utils_heartbeat(queue.queue);

        // Sit in the event loop performing work as it comes in.
        emscripten_exit_with_live_runtime();
      }) {

    // Make sure the thread has actually started before returning. This allows
    // subsequent code to assume the thread has already been spawned and not
    // worry about potential deadlocks where it holds a lock while proxying an
    // operation and meanwhile the main thread is blocked trying to acqure the
    // same lock so is never able to spawn the worker thread.
    //
    // Unfortunately, this solution would cause the main thread to deadlock on
    // itself, so for now assert that we are not on the main thread. In the
    // future, we could provide an asynchronous version of this utility that
    // calls a user callback once the worker has been started. This asynchronous
    // version would be safe to use on the main thread.
    assert(
      !emscripten_is_main_browser_thread() &&
      "cannot safely spawn dedicated workers from the main browser thread");
    {
      std::unique_lock<std::mutex> lock(mutex);
      cond.wait(lock, [&]() { return started; });
    }
  }

  // Kill the worker thread.
  ~ProxyWorker() {
    pthread_cancel(thread.native_handle());
    thread.join();
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
};

} // namespace emscripten
