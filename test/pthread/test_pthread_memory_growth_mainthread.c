// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten/em_js.h>
#include <stdbool.h>

// We want to test that JavaScript access in a pthread automatically updates the
// memory views if the heap has grown on the main thread meanwhile.
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

EM_JS(void, js_assert_initial_heap_state, (bool isWorker), {
  err(`Checking initial heap state on the ${isWorker ? 'worker' : 'main'} thread`);
  assert(HEAP8.length === 32 * 1024 * 1024, "start at 32MB");
});

EM_JS(void, js_assert_final_heap_state, (bool isWorker, const char* buffer, int finalHeapSize), {
  err(`Checking final heap state on the ${isWorker ? 'worker' : 'main'} thread`);
  assert(HEAP8.length > finalHeapSize, "end with >64MB");
  assert(HEAP8[buffer] === 42, "readable from JS");
});

_Atomic enum state {
  INITIAL_STATE,
  THREAD_CHECKED_INITIAL_STATE,
  FINAL_STATE
} state;

const char *_Atomic buffer;

void assert_initial_heap_state(bool is_worker) {
  assert(state == INITIAL_STATE);
  assert(buffer == NULL);
  js_assert_initial_heap_state(is_worker);
}

#define FINAL_HEAP_SIZE (64 * 1024 * 1024)

void assert_final_heap_state(bool is_worker) {
  assert(state == FINAL_STATE);
  assert(buffer != NULL);
  assert(*buffer == 42);
  js_assert_final_heap_state(is_worker, buffer, FINAL_HEAP_SIZE);
}

void *thread_start(void *arg) {
  assert_initial_heap_state(true);
  // Tell main thread that we checked the initial state and it can allocate.
  state = THREAD_CHECKED_INITIAL_STATE;
  // Wait for the main thread to allocate memory.
  while (state != FINAL_STATE);
  // Check the final heap state.
  assert_final_heap_state(true);
  return NULL;
}

char *alloc_beyond_initial_heap() {
  char *buffer = malloc(FINAL_HEAP_SIZE);
  assert(buffer);
  // Write value at the end of the buffer to check that any thread can access addresses beyond the initial heap size.
  buffer += FINAL_HEAP_SIZE - 1;
  *buffer = 42;
  return buffer;
}

int main() {
  // Check initial state in both threads before allocating more memory.
  assert_initial_heap_state(false);

  pthread_t thr;
  int res = pthread_create(&thr, NULL, thread_start, NULL);
  assert(res == 0);
  while (state != THREAD_CHECKED_INITIAL_STATE);

  // allocate more memory than we currently have, forcing a growth
  buffer = alloc_beyond_initial_heap();
  state = FINAL_STATE;

  assert_final_heap_state(false);
  res = pthread_join(thr, NULL);
  assert(res == 0);

  return 0;
}
