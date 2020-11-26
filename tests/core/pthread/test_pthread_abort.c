// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

void *thread_main(void *arg) {
  printf("thread_main\n");
  abort();
  printf("done thread_main\n");
}

int main() {
  pthread_t t;
  int rc = pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);
  printf("done!\n");
  return 0;
}
