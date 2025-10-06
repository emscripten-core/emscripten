// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <unistd.h>

#define NUM_THREADS 8

int numThreadsToCreateTotal = 50;

pthread_t thread[NUM_THREADS] = {};

volatile int counter = 0; // Shared data
pthread_mutex_t lock;

void sleepms(int msecs) {
  // Test two different variants of sleeping to verify
  // against bug https://bugzilla.mozilla.org/show_bug.cgi?id=1131757
#ifdef SPINLOCK_TEST
  double t0 = emscripten_get_now();
  double t1 = t0 + (double)msecs;
  while(emscripten_get_now() < t1)
    ;
#else
  usleep(msecs*1000);
#endif
}

void *ThreadMain(void *arg) {
  pthread_mutex_lock(&lock);
  int c = counter;
  sleepms(100); // Create contention on the lock.
  ++c;
  counter = c;
  pthread_mutex_unlock(&lock);
  pthread_exit(0);
}

void CreateThread(int i, int n) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 4*1024);
  int rc = pthread_create(&thread[i], &attr, ThreadMain, 0);
  if (rc != 0 || thread[i] == 0)
    printf("Failed to create thread!\n");
  pthread_attr_destroy(&attr);
}

int threadNum = 0;

bool WaitToJoin(double time, void *userData) {
  int threadsRunning = 0;
  // Join all threads.
  for (int i = 0; i < NUM_THREADS; ++i) {
    if (thread[i]) {
      void *status;
      int rc = pthread_join(thread[i], &status);
      if (rc == 0) {
        thread[i] = 0;
        if (threadNum < numThreadsToCreateTotal) {
          CreateThread(i, threadNum++);
          ++threadsRunning;
        }
      } else {
        ++threadsRunning;
      }
    }
  }
  if (!threadsRunning) {
    if (counter == numThreadsToCreateTotal)
      emscripten_outf("All threads finished. Counter = %d as expected", counter);
    else
      emscripten_errf("All threads finished, but counter = %d != %d!", counter, numThreadsToCreateTotal);
    assert(counter == 50);
    emscripten_force_exit(0);
    return false;
  }
  return true;
}

int main() {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&lock, &attr);

  pthread_mutex_lock(&lock);
  pthread_mutex_unlock(&lock);

  // Create new threads in parallel.
  for (int i = 0; i < NUM_THREADS; ++i) {
    CreateThread(i, threadNum++);
  }

  emscripten_set_timeout_loop(WaitToJoin, 100, 0);
  return 99;
}
