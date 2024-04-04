// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define _GNU_SOURCE

#include <pthread.h>
#include <emscripten/console.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

volatile int result = 0;

static void *thread2_start(void *arg) {
  emscripten_out("thread2_start!");
  ++result;
  return NULL;
}

static void *thread1_start(void *arg) {
  emscripten_out("thread1_start!");
  pthread_t thr;
  int rtn = pthread_create(&thr, NULL, thread2_start, NULL);
#ifdef SMALL_POOL
  assert(rtn != 0);
#else
  assert(rtn == 0);
  pthread_join(thr, NULL);
#endif
  return NULL;
}

#define DEFAULT_STACK_SIZE (64*1024)
int main() {
  pthread_t thr;
  pthread_create(&thr, NULL, thread1_start, NULL);

  pthread_attr_t attr;
  pthread_getattr_np(thr, &attr);
  size_t stack_size;
  void *stack_addr;
  pthread_attr_getstack(&attr, &stack_addr, &stack_size);
  printf("stack_size: %d, stack_addr: %p\n", (int)stack_size, stack_addr);
  assert(stack_size == DEFAULT_STACK_SIZE && stack_addr != NULL);

  pthread_join(thr, NULL);

  printf("done result=%d\n", result);
#ifdef SMALL_POOL
  assert(result == 0);
#else
  assert(result == 1);
#endif
  return 0;
}
