// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#define NUM_THREADS 8

unsigned int futexVal = 0;

unsigned int numAwoken = 0;

pthread_t wakingThread;

pthread_t waitingThreads[NUM_THREADS];

void *WakingThread(void *arg) {
  // Wake all threads waiting for the futex address
  emscripten_atomic_store_u32(&futexVal, 1);
  emscripten_futex_wake(&futexVal, INT_MAX);

  pthread_exit(0);
}

void *WaitingThread(void *arg) {
  // Last waiting thread creates the waking thread - this simplifies mutual synchronization needs
  if ((long)arg == NUM_THREADS-1) {
    pthread_create(&wakingThread, 0, WakingThread, 0);
  }

  // Each waiting thread waits until wake thread changes the value at the wait address
  while (emscripten_atomic_load_u32(&futexVal) == 0) {
    emscripten_futex_wait(&futexVal, 0, INFINITY);
  }

  // Tally up the number of awoken threads - last one to wake up signals test success
  uint32_t old = emscripten_atomic_add_u32(&numAwoken, 1);
  if (old + 1 == NUM_THREADS) {
    emscripten_force_exit(0);
  }

  return 0;
}

int main() {
  for (intptr_t i = 0; i < NUM_THREADS; ++i) {
    pthread_create(waitingThreads+i, 0, WaitingThread, (void*)i);
  }
  emscripten_exit_with_live_runtime();
}
