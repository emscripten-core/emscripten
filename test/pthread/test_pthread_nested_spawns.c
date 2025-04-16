// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <pthread.h>

int result = 0;

static void *thread2_func(void *vptr_args) {
  puts("c");
  result = 1;
  return NULL;
}

static void *thread_func(void *vptr_args) {
  pthread_t thread;
  puts("b");
  pthread_create(&thread, NULL, thread2_func, NULL);
  pthread_join(thread, NULL);
  return NULL;
}

int main(void) {
  pthread_t thread;
  puts("a");
  pthread_create(&thread, NULL, thread_func, NULL);
  pthread_join(thread, NULL);

  assert(result == 1);
  return 0;
}
