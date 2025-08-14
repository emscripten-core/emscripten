// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten/em_js.h>
#include <stdbool.h>

// We want to test that JavaScript access in the main thread automatically
// updates the memory views if the heap has grown from the pthread meanwhile.
//
// Checking this correctly is somewhat tricky because a lot of Emscripten APIs
// access heap on their own, so the memory views might be updated by them before
// we explicitly check the state in our own JS routines below, which leads to
// false positives (test passing even though in isolation the heap access in our
// own JS is not the one updating the memory views).
//
// To test it in isolation, we need to use only EM_JS - which is as close to a
// pure JS call as we can get - and not EM_ASM, pthread_join, proxying etc., not
// even `puts`/`printf` for logging - as all of those access the heap from JS on
// their own and might update the memory views too early. Not using standard
// proxying mechanisms also means we need to drop down all the way to raw
// atomics.

EM_JS(void, assert_initial_heap_state, (bool isWorker), {
  dbg(`Checking initial heap state on the ${isWorker ? 'worker' : 'main'} thread`);
  assert(HEAP8.length === 32 * 1024 * 1024, "start at 32MB");
});

EM_JS(void, js_assert_final_heap_state, (bool isWorker, const char* buffer, int finalHeapSize), {
  dbg(`Checking final heap state on the ${isWorker ? 'worker' : 'main'} thread`);
  assert(HEAP8.length > finalHeapSize, "end with >64MB");
  assert(HEAP8[buffer] === 42, "readable from JS");
});

#define FINAL_HEAP_SIZE (64 * 1024 * 1024)

char *alloc_beyond_initial_heap() {
  char *buffer = malloc(FINAL_HEAP_SIZE);
  assert(buffer);
  // Write value at the end of the buffer to check that any thread can access addresses beyond the initial heap size.
  buffer += FINAL_HEAP_SIZE - 1;
  *buffer = 42;
  return buffer;
}

const char *_Atomic buffer = NULL;

void assert_final_heap_state(bool is_worker) {
  assert(buffer != NULL);
  assert(*buffer == 42);
  js_assert_final_heap_state(is_worker, buffer, FINAL_HEAP_SIZE);
}

void *thread_start(void *arg) {
  assert_initial_heap_state(true);
  // allocate more memory than we currently have, forcing a growth
  buffer = alloc_beyond_initial_heap();
  assert_final_heap_state(true);
  return NULL;
}

int main() {
  assert_initial_heap_state(false);

  pthread_t thr;
  int res = pthread_create(&thr, NULL, thread_start, NULL);
  assert(res == 0);

  while (!buffer);

  assert(*buffer == 42); // should see the value the thread wrote
  assert_final_heap_state(false);

  res = pthread_join(thr, NULL);
  assert(res == 0);

  return 0;
}
