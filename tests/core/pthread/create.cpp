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
    // wait for a change, so we see interleaved processing.
    int last = ++sum;
    while (sum.load() == last) {}
  }
  pthread_exit((void*)TOTAL);
}

pthread_t thread[NUM_THREADS];

void CreateThread(long i)
{
  int rc = pthread_create(&thread[i], nullptr, ThreadMain, (void*)i);
  assert(rc == 0);
}

void mainn() {
  static int main_adds = 0;
  int worker_adds = sum++ - main_adds++;
  printf("main iter %d : %d\n", main_adds, worker_adds);
  if (worker_adds == NUM_THREADS * TOTAL) {
    printf("done!\n");
#ifndef ALLOW_SYNC
    emscripten_cancel_main_loop();
#endif
    exit(0);
  }
}

int main() {
  // Create initial threads.
  for(long i = 0; i < NUM_THREADS; ++i) {
    CreateThread(i);
  }

  // if we don't allow sync pthread creation, the event loop must be reached for
  // the worker to start up.
#ifndef ALLOW_SYNC
  emscripten_set_main_loop(mainn, 0, 0);
#else
  while (1) mainn();
#endif
  return 0;
}
