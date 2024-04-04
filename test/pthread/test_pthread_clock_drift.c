// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <math.h>
#include <stdio.h>

volatile int threadStarted = 0;
volatile int timeReceived = 0;
volatile double mainThreadTime;

void wait(volatile int *address) {
  int state = emscripten_atomic_load_u32((void*)address);
  while (state == 0) {
    state = emscripten_atomic_load_u32((void*)address);
  }
}

void wake(volatile int *address) {
  emscripten_atomic_store_u32((void*)address, 1);
}

void *thread_main(void *arg) {
  wake(&threadStarted);
  wait(&timeReceived);
  double pthreadTime = emscripten_get_now();
  double timeDifference = pthreadTime - mainThreadTime;
  printf("Time difference between pthread and main thread is %f msecs.\n", timeDifference);

  // The time difference here should be well less than 1 msec, but test against
  // 200msecs to be super-sure.
  assert(fabs(timeDifference) < 200);
  emscripten_force_exit(0);
  return 0;
}

void busy_sleep(double msecs) {
  double end = emscripten_get_now() + msecs;
  while (emscripten_get_now() < end) {
    // busy loop
  }
}

int main()
{
  // Cause a one second delay between main() and pthread start that might have a
  // chance to drift the wallclocks on emscripten_get_now().
  busy_sleep(1000);

  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  wait(&threadStarted);
  mainThreadTime = emscripten_get_now();
  wake(&timeReceived);

  emscripten_exit_with_live_runtime();
}
