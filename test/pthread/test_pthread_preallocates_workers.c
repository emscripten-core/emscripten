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
#include <emscripten/em_asm.h>
#include <emscripten/console.h>

pthread_t threads[5];

static void *thread_start(void *arg)
{
  // This should be long enough for threads to pile up.
  long idx = (long)arg;
  printf("Starting thread %ld\n", idx);
  while (1) {
    sleep(1);
  }
  printf("Finishing thread %ld\n", idx);
  pthread_exit((void*)0);
}

void CreateThread(intptr_t idx) {
  emscripten_outf("Main: Spawning thread %ld...", idx);
  int rc = pthread_create(&threads[idx], NULL, thread_start, (void*)idx);
  assert(rc == 0);
}

int main()
{
  // This test should be run with a prewarmed pool of size 4. None
  // of the threads are allocated yet.
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 4);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);

  CreateThread(0);

  // We have one running thread, allocated on demand.
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 3);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 1);

  for (int i = 1; i < 5; ++i) {
    CreateThread(i);
  }

  // All the preallocated workers should be used.
  // We can't join the threads or we'll hang forever. The main thread
  // won't give up the thread to let the 5th thread be created. This is
  // solved in non-test cases by using PROXY_TO_PTHREAD, but we can't
  // do that here since we need to eval the length of the various pthread
  // arrays.
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 5);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 0);

  return 0;
}
