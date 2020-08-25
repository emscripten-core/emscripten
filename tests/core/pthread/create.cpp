// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <emscripten.h>

#include <atomic>

#define NUM_THREADS 2
#define TOTAL 100

static std::atomic<int> sum;

void *ThreadMain(void *arg) {
  for (int i = 0; i < TOTAL; i++) {
    sum++;
    // wait for a change, so we see interleaved processing.
    int last = sum.load();
    while (sum.load() == last) {}
  }
  pthread_exit((void*)TOTAL);
}

pthread_t thread[NUM_THREADS];

void CreateThread(int i)
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  static int counter = 1;
  int rc = pthread_create(&thread[i], &attr, ThreadMain, (void*)i);
  assert(rc == 0);
  pthread_attr_destroy(&attr);
}

void mainn() {
  static int main_adds = 0;
  int worker_adds = sum.load() - main_adds;
  sum++;
  main_adds++;
  printf("main iter %d : %d\n", main_adds, worker_adds);
  if (worker_adds == NUM_THREADS * TOTAL) {
    printf("done!\n");
#ifndef POOL
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
  }
}

int main() {
  // Create initial threads.
  for(int i = 0; i < NUM_THREADS; ++i) {
    CreateThread(i);
  }

  // Without a pool, the event loop must be reached for the worker to start up.
#ifndef POOL
  emscripten_set_main_loop(mainn, 0, 0);
#else
  while (1) mainn();
#endif
}
