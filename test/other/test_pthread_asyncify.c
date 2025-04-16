// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/console.h>

void* thread_main(void* arg) {
  printf("thread main\n");
  emscripten_sleep(10);
  printf("done sleep\n");
  return NULL;
}

int main() {
  printf("main\n");
  pthread_t thread;
  int rc = pthread_create(&thread, NULL, thread_main, NULL);
  assert(rc == 0);
  rc = pthread_join(thread, NULL);
  assert(rc == 0);
  printf("done pthread_join\n");
  return 0;
}
