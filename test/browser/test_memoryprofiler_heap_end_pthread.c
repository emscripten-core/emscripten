/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Tests that the memory profiler (--memoryprofiler) tracks the end of the
// dynamic memory area (i.e. the sbrk() limit) correctly when it is a pthread
// that grows it, in which case the notification from sbrk() gets forwarded to
// the main thread.

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/heap.h>

// How many times, at 10 msec intervals, to check for the notification from the
// pthread before giving up.
#define MAX_TRIES 500

EM_JS(double, get_heap_end, (void), {
  return emscriptenMemoryProfiler.heapEnd;
});

EM_JS(void, check_heap_end, (double expected), {
  var actual = emscriptenMemoryProfiler.heapEnd;
  assert(actual == expected, `heapEnd is ${actual}, expected ${expected}`);
});

void* thread_main(void* arg) {
  // Grow the dynamic memory area.
  void* p = malloc(4 * 1024 * 1024);
  assert(p);
  return p;
}

void check(void* arg) {
  static int tries = 0;
  // Read the sbrk limit directly, since calling sbrk(0) would itself report
  // the limit to the memory profiler.
  double expected = (double)*emscripten_get_sbrk_ptr();
  // The notification from the pthread arrives asynchronously on the main
  // thread, so poll for it rather than rely on it having arrived already.
  if (get_heap_end() != expected && ++tries < MAX_TRIES) {
    emscripten_async_call(check, NULL, 10);
    return;
  }
  check_heap_end(expected);
  emscripten_force_exit(0);
}

int main(void) {
  uintptr_t old_brk = *emscripten_get_sbrk_ptr();

  pthread_t thread;
  int rc = pthread_create(&thread, NULL, thread_main, NULL);
  assert(rc == 0);
  rc = pthread_join(thread, NULL);
  assert(rc == 0);
  assert(*emscripten_get_sbrk_ptr() > old_brk);

  emscripten_async_call(check, NULL, 0);
  return 0;
}
