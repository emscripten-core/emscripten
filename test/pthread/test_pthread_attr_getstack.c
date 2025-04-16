// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define _GNU_SOURCE

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void TestStack() {
  pthread_attr_t attr;
  int rc;
  void *stbase;
  size_t stsize;
  char dummy;

  rc = pthread_attr_init(&attr);
  assert(rc == 0);

  rc = pthread_getattr_np(pthread_self(), &attr);
  assert(rc == 0);

  rc = pthread_attr_getstack(&attr, &stbase, &stsize);
  assert(rc == 0);

  printf("size=%zu base=%p base+size=%p dummy=%p\n", stsize, stbase, (char*)stbase + stsize, &dummy);

  assert(&dummy >= (char*)stbase);
  assert(&dummy <= (char*)stbase + stsize);
}

void *ThreadMain(void *arg) {
  TestStack();
  return NULL;
}

int main() {
  // Run TestStack both on the main thread and on a secondary thread
  TestStack();
  pthread_t thread;
  int rc;

  rc = pthread_create(&thread, NULL, ThreadMain, NULL);
  assert(rc == 0);

  rc = pthread_join(thread, NULL);
  assert(rc == 0);

  printf("done\n");
  return 0;
}
