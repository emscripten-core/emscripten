// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include <unistd.h> // usleep
#include <assert.h>

volatile int counter = 0; // Shared data
volatile int lock = 0; // spinlock "mutex" variable

void *ThreadMain(void *arg) {
  printf("Thread started.\n");
  for (int i = 0; i < 100; ++i) {
#ifdef USE_EMSCRIPTEN_INTRINSICS
    while(emscripten_atomic_exchange_u32((void*)&lock, 1))
#else
    while(__sync_lock_test_and_set(&lock, 1))
#endif
    {
      /*nop*/;
    }
    int c = counter;
    usleep(5 * 1000); // Create contention on the lock.
    ++c;
    counter = c;
#ifdef USE_EMSCRIPTEN_INTRINSICS
    emscripten_atomic_store_u32((void*)&lock, 0);
#else
    __sync_lock_release(&lock);
#endif
  }
  printf("Thread done.\n");
  pthread_exit(0);
}

#define NUM_THREADS 8

int main() {
  pthread_t thread[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_create(&thread[i], NULL, ThreadMain, 0);
    if (rc != 0 || thread[i] == 0) {
      printf("Failed to create thread!\n");
    }
  }

  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_join(thread[i], 0);
    assert(rc == 0);
  }

  printf("counter: %d\n", counter);
  assert(counter == 800);
  return 0;
}
