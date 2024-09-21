// Copyright 2019 The Emscripten Authors.  All rights reserved.
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
#include <emscripten.h>

pthread_t threads[50];

static void *thread_start(void *arg) {
  // This thread quits immediately...
  pthread_exit((void*)0);
}

void CreateThread(intptr_t idx) {
  int rc = pthread_create(&threads[idx], NULL, thread_start, (void*)idx);
  assert(rc == 0);
}

void JoinThread(int idx) {
  int rc = pthread_join(threads[idx], NULL);
  assert(rc == 0);
}

int main() {
  // This test should be run with a prewarmed pool of size 50. They should be fully allocated.
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 50);

  double total = 0;
  for (int i = 0; i < 10; ++i) {
    double t1 = emscripten_get_now();
    for (int j = 0; j < 50; ++j) {
      CreateThread(j);
    }
    double t2 = emscripten_get_now();
    printf("Took %f ms to allocate 50 threads.\n", t2 - t1);
    total += (t2 - t1);
    // Join all the threads to clear the queue..
    for (int j = 0; j < 50; ++j) {
      JoinThread(j);
    }
  }

  printf("Final average %f ms.\n", total / 10.0);
  return 0;
}
