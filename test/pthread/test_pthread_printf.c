// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

void *ThreadMain(void *arg) {
  printf("Hello from thread, string: %s, int: %d, double: %g\n", "str", 5, 42.0);
  return 0;
}

int main() {
  pthread_t thread;
  int rc = pthread_create(&thread, NULL, ThreadMain, 0);
  assert(rc == 0);

  rc = pthread_join(thread, NULL);
  assert(rc == 0);

  printf("The thread should print 'Hello from thread, string: str, int: 5, double: 42.0'\n");
  return 0;
}
