// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static _Thread_local int thread_local = 0;

void *thread_start(void *arg) {
  thread_local = 1;
  assert(thread_local == 1);
  printf("thread_local=%d\n", thread_local);
  return NULL;
}

int main() {
  thread_local = 2;
  pthread_t thread;
  int rc;

  rc = pthread_create(&thread, NULL, thread_start, NULL);
  assert(rc == 0);

  rc = pthread_join(thread, NULL);
  assert(rc == 0);

  printf("thread_local=%d\n", thread_local);
  assert(thread_local == 2);
  printf("done\n");
  return 0;
}
