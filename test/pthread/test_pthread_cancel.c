// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <emscripten/console.h>

_Atomic bool done_cleanup = false;

void cleanup_handler(void *arg) {
  long a = (long)arg;
  emscripten_outf("Called clean-up handler with arg %ld", a);
  assert(a == 42);
  done_cleanup = true;
}

void *thread_start(void *arg) {
  pthread_cleanup_push(cleanup_handler, (void*)42);
  emscripten_out("Thread started!");
  while (1) {
    pthread_testcancel();
  }
  assert(false);
  pthread_cleanup_pop(0);
}

int main() {
  pthread_t thr;
  int s = pthread_create(&thr, NULL, thread_start, (void*)0);
  assert(s == 0);
  emscripten_out("Canceling thread..");
  s = pthread_cancel(thr);
  assert(s == 0);

  while (!done_cleanup) {
  }

  emscripten_outf("After canceling, cleanup complete");
  return 0;
}
