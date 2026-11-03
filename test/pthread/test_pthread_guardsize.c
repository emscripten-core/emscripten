// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_main_default(void* arg) {
  pthread_attr_t attr;
  size_t guard_size = 0;
  assert(pthread_getattr_np(pthread_self(), &attr) == 0);
  assert(pthread_attr_getguardsize(&attr, &guard_size) == 0);
  printf("default guard_size: %zu\n", guard_size);
  assert(guard_size == 8192);
  return NULL;
}

void* thread_main_custom(void* arg) {
  pthread_attr_t attr;
  size_t guard_size = 0;
  assert(pthread_getattr_np(pthread_self(), &attr) == 0);
  assert(pthread_attr_getguardsize(&attr, &guard_size) == 0);
  printf("custom guard_size: %zu\n", guard_size);
  assert(guard_size == 16384);
  return NULL;
}

void* thread_main_zero(void* arg) {
  pthread_attr_t attr;
  size_t guard_size = 0;
  assert(pthread_getattr_np(pthread_self(), &attr) == 0);
  assert(pthread_attr_getguardsize(&attr, &guard_size) == 0);
  printf("zero guard_size: %zu\n", guard_size);
  assert(guard_size == 0);
  return NULL;
}

int main() {
  pthread_t t1, t2, t3;
  pthread_attr_t attr;

  assert(pthread_create(&t1, NULL, thread_main_default, NULL) == 0);
  assert(pthread_join(t1, NULL) == 0);

  assert(pthread_attr_init(&attr) == 0);
  assert(pthread_attr_setguardsize(&attr, 16384) == 0);
  assert(pthread_create(&t2, &attr, thread_main_custom, NULL) == 0);
  assert(pthread_join(t2, NULL) == 0);
  assert(pthread_attr_destroy(&attr) == 0);

  assert(pthread_attr_init(&attr) == 0);
  assert(pthread_attr_setguardsize(&attr, 0) == 0);
  assert(pthread_create(&t3, &attr, thread_main_zero, NULL) == 0);
  assert(pthread_join(t3, NULL) == 0);
  assert(pthread_attr_destroy(&attr) == 0);

  printf("done\n");
  return 0;
}
