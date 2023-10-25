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
#include <emscripten/console.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
_Atomic long res = 43;
_Atomic int started = false;

static void cleanup_handler(void *arg)
{
  long a = (long)arg;
  emscripten_outf("Called clean-up handler with arg %ld", a);
  res -= a;
}

static void *thread_start(void *arg) {
  // Setup thread for async cancelation only
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  pthread_cleanup_push(cleanup_handler, (void*)42);

  emscripten_out("Thread started!");

  // Signal the main thread that are started
  started = true;

  // This mutex is locked by the main thread so this call should never return.
  // pthread_mutex_lock is not a cancellation point so deferred cancellation
  // won't work here, async cancelation should.
  pthread_mutex_lock(&mutex);

  assert(false && "pthread_mutex_lock returned!");
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
  while (res != 1) {
    sched_yield();
  }

  emscripten_out("Joining thread..");
  s = pthread_join(thr, NULL);
  assert(s == 0);
  emscripten_out("done");
  return 0;
}
