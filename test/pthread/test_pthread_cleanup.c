// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

// Stores/encodes the results of calling to cleanup handlers.
int32_t cleanup_state = 1;

static void cleanup_handler1(void *arg) {
  cleanup_state <<= 2;
  // Perform non-commutative arithmetic to a global var that encodes the cleanup stack order ops.
  cleanup_state *= (intptr_t)arg;
  printf("Called clean-up handler 1 with arg %p (state=%d)\n", arg, cleanup_state);
}

static void cleanup_handler2(void *arg) {
  cleanup_state <<= 3;
  // Perform non-commutative arithmetic to a global var that encodes the cleanup stack order ops.
  cleanup_state *= (intptr_t)arg;
  printf("Called clean-up handler 2 with arg %p (state=%d)\n", arg, cleanup_state);
}

static void *thread_start1(void *arg) {
  pthread_cleanup_push(cleanup_handler1, (void*)(42 + (long)arg*100));
  pthread_cleanup_push(cleanup_handler2, (void*)(69 + (long)arg*100));
  pthread_cleanup_pop((int)(intptr_t)arg);
  pthread_cleanup_pop((int)(intptr_t)arg);
  pthread_exit(0);
}

static void *thread_start2(void *arg) {
  pthread_cleanup_push(cleanup_handler1, (void*)52);
  pthread_cleanup_push(cleanup_handler2, (void*)79);
  if (arg)
    pthread_exit(0);
  pthread_cleanup_pop(0);
  pthread_cleanup_pop(0);
  return 0;
}

static void *thread_start3(void *arg) {
  pthread_cleanup_push(cleanup_handler1, (void*)62);
  pthread_cleanup_push(cleanup_handler2, (void*)89);
  for (;;) {
     pthread_testcancel();
  }
  pthread_cleanup_pop(0);
  pthread_cleanup_pop(0);
  pthread_exit(0);
}

pthread_t thr[4];

int main() {
  int result = 0;

  pthread_cleanup_push(cleanup_handler1, (void*)9998);
  pthread_cleanup_push(cleanup_handler1, (void*)9999);

  int s = pthread_create(&thr[0], NULL, thread_start1, (void*)0);
  assert(s == 0);
  pthread_join(thr[0], 0);
  s = pthread_create(&thr[1], NULL, thread_start1, (void*)1);
  assert(s == 0);
  pthread_join(thr[1], 0);
  s = pthread_create(&thr[2], NULL, thread_start2, (void*)1);
  assert(s == 0);
  pthread_join(thr[2], 0);
// TODO
//   s = pthread_create(&thr[3], NULL, thread_start3, (void*)1);
//   assert(s == 0);
//   s = pthread_cancel(thr[3]);
//   assert(s == 0);
  pthread_cleanup_pop(1);
  printf("Cleanup state variable: %d\n", cleanup_state);
  assert(cleanup_state == 907640832);

  pthread_cleanup_pop(1);
  exit(EXIT_SUCCESS);
}

/*
"Called clean-up handler 1 with arg 9999" b.js line 446 > eval:1
"exit(0) called, but noExitRuntime, so not exiting" b.html:1245
"exit(0) called, but noExitRuntime, so not exiting" b.html:1245
"Called clean-up handler 2 with arg 79" b.js line 446 > eval:1
"Called clean-up handler 2 with arg 169" b.js line 446 > eval:1
"Called clean-up handler 1 with arg 52" b.js line 446 > eval:1
"Called clean-up handler 1 with arg 142"
*/
