// Copyright 2021 The Emscripten Authors.  All rights reserved.
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

pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;

_Atomic bool th_cancelled = false;
_Atomic int result = 0;

static void cleanup_handler(void *arg) {
  emscripten_outf("Called clean-up handler with arg %p", arg);
  result = (intptr_t)(arg);
  assert(result == 42);

  pthread_mutex_unlock(&mutex);
  pthread_barrier_wait(&barrier);
}

static void *thread_start(void *arg) {
  pthread_cleanup_push(cleanup_handler, (void*)42);
  emscripten_outf("Thread started!");
  pthread_mutex_lock(&mutex);
  pthread_barrier_wait(&barrier);

  int ret = 0;
  do {
    emscripten_outf("Waiting on conditional variable");
    ret = pthread_cond_wait(&condvar, &mutex);
  } while (ret == 0 && !th_cancelled);

  if (ret != 0) {
    emscripten_outf("Cond wait failed ret: %d", ret);
  }

  assert(false); // Shouldn't ever reach here.
  pthread_cleanup_pop(0);

  pthread_mutex_unlock(&mutex);
  pthread_barrier_wait(&barrier);
  return NULL;
}

int main() {
  pthread_barrier_init(&barrier, NULL, 2);

  pthread_t thr;
  int s = pthread_create(&thr, NULL, thread_start, (void*)0);
  assert(s == 0);
  emscripten_outf("Thread created");

  pthread_barrier_wait(&barrier);

  // Lock mutex to ensure that thread is waiting
  pthread_mutex_lock(&mutex);

  emscripten_outf("Canceling thread..");
  s = pthread_cancel(thr);
  assert(s == 0);
  th_cancelled = true;
  pthread_mutex_unlock(&mutex);

  emscripten_outf("Main thread waiting for side-thread");
  pthread_barrier_wait(&barrier);
  pthread_barrier_destroy(&barrier);

  emscripten_outf("Test finished result: %d", result);
  assert(result == 42);
  return 0;
}
