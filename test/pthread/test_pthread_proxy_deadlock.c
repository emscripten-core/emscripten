// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <emscripten/console.h>
#include <emscripten/heap.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

// In the actual implementation of malloc the system queue may be executed
// non-deterministically if malloc is waiting on a mutex. This wraps malloc and
// executes the system queue during every allocation to make the behavior
// deterministic.
void *malloc(size_t size) {
  if (emscripten_is_main_runtime_thread()) {
    emscripten_proxy_execute_queue(emscripten_proxy_get_system_queue());
  }
  void *ptr = emscripten_builtin_malloc(size);
  return ptr;
}

void task(void* arg) {
  emscripten_out("task\n");
}

int main() {
  // Tests for a deadlock scenario that can occur when sending a task.
  // The sequence of events is:
  // 1. Sending a task locks the queue.
  // 2. Allocating a new task queue calls malloc.
  // 3. Malloc then attempts to execute and lock the already-locked queue,
  //    causing a deadlock.
  // This test ensures our implementation prevents this re-entrant lock.
  emscripten_proxy_async(emscripten_proxy_get_system_queue(), pthread_self(), task, NULL);
  emscripten_out("done\n");
}
