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

bool should_quit = false;
pthread_t looper;

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

void run_on_looper(void* arg) {
  emscripten_out("run_on_looper\n");
  should_quit = true;
}

void* looper_main(void* arg) {
  while (!should_quit) {
    emscripten_proxy_execute_queue(emscripten_proxy_get_system_queue());
    sched_yield();
  }
  return NULL;
}

int main() {
  pthread_create(&looper, NULL, looper_main, NULL);
  emscripten_proxy_async(emscripten_proxy_get_system_queue(), looper, run_on_looper, NULL);
  pthread_join(looper, NULL);
  emscripten_out("done\n");
}
