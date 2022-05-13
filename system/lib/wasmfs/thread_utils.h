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
        // Process the queue in case any work came in while we were starting up,
        // then sit in the event loop performing work as it comes in.
        queue.execute();
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
