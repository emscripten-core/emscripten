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
#include <emscripten/threading.h>
#include <vector>

bool alive[5] = {0};
pthread_t threads[5];

static void *thread_start(void *arg)
{
  // This should be long enough for threads to pile up.
  int idx = (int)arg;
  while (alive[idx]) {
    sleep(1);
  }
  pthread_exit((void*)0);
}

int main()
{
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  // This test should be run with a prewarmed pool of size 8. 4 of the
  // preallocated workers should have been used already.
  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 4);
  // This test should be run with a prepopulated pool of size 4.
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 4);

  EM_ASM(out('Main: Spawning thread...'), NULL);
  alive[0] = true;
  int rc = pthread_create(&threads[0], NULL, thread_start, (void*)0);
  assert(rc == 0);
  EM_ASM(out('Main: Waiting for thread to join.'));
  int result = 0;

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 4);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 1);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 3);

  // We can join here because 4 threads were preallocated.
  alive[0] = false;
  rc = pthread_join(threads[0], NULL);
  assert(rc == 0);
  EM_ASM(out('Main: Thread joined with result: '+$0+'.'), result);

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 4);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 4);

  for (int i = 0; i < 5; ++i) {
    alive[i] = true;
    EM_ASM(out('Main: Spawning thread '+$0+'...'), i);
    int rc = pthread_create(&threads[i], NULL, thread_start, (void*)i);
    assert(rc == 0);
  }

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 3);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 5);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 0);

  // We can join the first 4 threads, as they were preallocated.
  for (int i = 0; i < 4; ++i) {
    alive[i] = false;
    rc = pthread_join(threads[i], NULL);
  }
  // Mark the last thread to be killed.
  alive[4] = false;

  // We can't join the last one or we'll hang forever. The main thread
  // won't give up the thread to let the 5th thread be created. This is
  // solved in non-test cases by using PROXY_TO_PTHREAD, but we can't
  // do that here since we need to eval the length of the various pthread
  // arrays. We don't know if the last thread is done or not when we are
  // finished.
  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 3);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) <= 1);
  int unused_workers = EM_ASM_INT(return PThread.unusedWorkers.length);
  assert(unused_workers == 4 || unused_workers == 5);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
