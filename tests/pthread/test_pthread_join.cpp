// Copyright 2015 The Emscripten Authors.  All rights reserved.
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

int fib(int n)
{
  if (n <= 0) return 0;
  if (n == 1) return 1;
  return fib(n-1) + fib(n-2);
}

static void *thread_start(void *arg)
{
  int n = (int)arg;
  EM_ASM(out('Thread: Computing fib('+$0+')...'), n);
  int fibn = fib(n);
  EM_ASM(out('Thread: Computation done. fib('+$0+') = '+$1+'.'), n, fibn);
  pthread_exit((void*)fibn);
}

int main()
{
  // Test existence of nanosleep(), https://github.com/emscripten-core/emscripten/issues/4578
  struct timespec ts = { 1, 0 };
  nanosleep(&ts, 0);

  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(6765);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  pthread_t thr;

  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 8); // This test should be run with a prepopulated pool of size 8.

  int n = 20;
  EM_ASM(out('Main: Spawning thread to compute fib('+$0+')...'), n);
  int s = pthread_create(&thr, NULL, thread_start, (void*)n);
  assert(s == 0);
  EM_ASM(out('Main: Waiting for thread to join.'));
  int result = 0;

  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 1);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 7);

  s = pthread_join(thr, (void**)&result);
  assert(s == 0);
  EM_ASM(out('Main: Thread joined with result: '+$0+'.'), result);

  assert(EM_ASM_INT(return PThread.runningWorkers.length) == 0);
  assert(EM_ASM_INT(return PThread.unusedWorkers.length) == 8);

#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}

