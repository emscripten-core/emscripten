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

int fib(int n)
{
  if (n <= 0) return 0;
  if (n == 1) return 1;
  return fib(n-1) + fib(n-2);
}

static void *thread_start(void *arg)
{
  int n = 20;
  EM_ASM(out('Thread: Computing fib('+$0+')...'), n);
  int fibn = fib(n);
  EM_ASM(out('Thread: Computation done. fib('+$0+') = '+$1+'.'), n, fibn);
  pthread_exit((void*)fibn);
}

pthread_t threads[4];

void CreateThread(int i) {
  int rc = pthread_create(&threads[i], NULL, thread_start, NULL);
  assert(rc == 0);
}


int main()
{
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(6765);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  // This test should be run with a prewarmed pool of size 4. all 4 of the
  // preallocated workers should have been used already.
  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 0);
  // This test should be run with a prepopulated pool of size 4.
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 4);

  int n = 20;
  EM_ASM(out('Main: Spawning thread to compute fib('+$0+')...'), n);
  CreateThread(0);
  EM_ASM(out('Main: Waiting for thread to join.'));
  int result = 0;

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 1);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 3);

  int s = pthread_join(threads[0], (void**)&result);
  assert(s == 0);
  EM_ASM(out('Main: Thread joined with result: '+$0+'.'), result);

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 4);

  for (int i = 0; i < 4; ++i) {
    EM_ASM(out('Main: Spawning thread '+$0+' to compute fib('+$1+')...'), i, n);
    CreateThread(i);
  }

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 4);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 0);

  // Joining them all should result in 4 in the unused state.
  for (int i = 0; i < 4; ++i) {
    EM_ASM(out('Main: Waiting for thread '+$0), i, n);
    s = pthread_join(threads[i], (void**)&result);
    assert(s == 0);
  }

  assert(EM_ASM_INT(return PThread.preallocatedWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 4);

#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}
