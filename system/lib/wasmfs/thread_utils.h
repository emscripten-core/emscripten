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

namespace emscripten {

// Helper class for synchronously proxying work to a dedicated worker thread,
// including where the work is asynchronous.
class ProxyWorker {
  // The queue we use to proxy work and the dedicated worker.
  ProxyingQueue queue;
  std::thread thread;

public:
  // Spawn the worker thread.
  ProxyWorker()
    : queue(), thread([&]() {
        // Sometimes the main thread is spinning, waiting on a WasmFS lock held
        // by a thread trying to proxy work to this dedicated worker. In that
        // case, the proxying message won't be relayed by the main thread and
        // the system will deadlock. This heartbeat ensures that proxying work
        // eventually gets done so the thread holding the lock can make forward
        // progress even if the main thread is blocked.
        //
        // TODO: Remove this once we can postMessage directly between workers
        // without involving the main thread.
        //
        // Note that this requires adding _emscripten_proxy_execute_queue to
        // EXPORTED_FUNCTIONS.
        EM_ASM({
          var intervalID =
            setInterval(() => {
              if (ABORT) {
                clearInterval(intervalID);
              } else {
                _emscripten_proxy_execute_queue($0);
              }
            }, 50);
          }, queue.queue);

        // Sit in the event loop performing work as it comes in.
        emscripten_exit_with_live_runtime();
      }) {}

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
