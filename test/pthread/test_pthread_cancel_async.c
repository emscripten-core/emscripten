// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/console.h>
#else
void emscripten_out(const char* msg) {
  printf("%s\n", msg);
}

void emscripten_outf(const char* msg, ...) {
  printf("%s\n", msg);
}
#endif

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
_Atomic bool started = false;
_Atomic bool timedlock_returned = false;
_Atomic bool done_cleanup = false;

void cleanup_handler(void *arg) {
  long a = (long)arg;
  emscripten_outf("Called clean-up handler with arg %ld", a);
  assert(a == 42);
  done_cleanup = true;
}

void *thread_start(void *arg) {
  // Setup thread for async cancellation only
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  pthread_cleanup_push(cleanup_handler, (void*)42);

  emscripten_out("Thread started!");

  // Signal the main thread that are started
  started = true;

  // At least under musl, async cancellation also does not work for
  // pthread_mutex_lock so this call to pthread_mutex_timedlock should always
  // timeout.
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 2;
  int rc = pthread_mutex_timedlock(&mutex, &ts);
  timedlock_returned = true;
  assert(rc == ETIMEDOUT);
  emscripten_out("pthread_mutex_timedlock timed out");

  pthread_testcancel();

  assert(false && "pthread_testcancel returned!");
  pthread_cleanup_pop(0);
}

int main() {
  pthread_mutex_lock(&mutex);

  emscripten_out("Starting thread..");
  pthread_t thr;
  int s = pthread_create(&thr, NULL, thread_start, (void*)0);
  assert(s == 0);
  // Busy wait until thread is started
  while (!started) {
    sched_yield();
  }

  emscripten_out("Canceling thread..");
  s = pthread_cancel(thr);
  assert(s == 0);
  // Busy wait until thread cancel handler has been run
  while (!done_cleanup) {
    sched_yield();
  }

  emscripten_out("Joining thread..");
  s = pthread_join(thr, NULL);
  assert(s == 0);
  assert(timedlock_returned);
  emscripten_out("done");
  return 0;
}
