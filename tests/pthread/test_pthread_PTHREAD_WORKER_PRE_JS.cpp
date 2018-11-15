// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>

#include <atomic>

#if !defined(__EMSCRIPTEN_PTHREADS__) || __EMSCRIPTEN_PTHREADS__ != 1
#error __EMSCRIPTEN_PTHREADS__ should have been defined to be equal to 1 when building with pthreads support enabled!
#endif

#define NUM_THREADS 8

std::atomic<int> atomicSum;

void *ThreadMain(void *arg)
{
  atomicSum += EM_ASM_INT({
    return Module["something"];
  });
  pthread_exit(0);
}

void RunThreads()
{  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setstacksize(&attr, 4*1024);

  printf("Main thread has thread ID %d\n", (int)pthread_self());
  assert(pthread_self() != 0);

  pthread_t thread[NUM_THREADS];

  for(int i = 0; i < NUM_THREADS; ++i)
  {
    int rc = pthread_create(&thread[i], &attr, ThreadMain, NULL);
    assert(rc == 0);
  }

  pthread_attr_destroy(&attr);

  for(int i = 0; i < NUM_THREADS; ++i)
  {
    int status = 1;
    int rc = pthread_join(thread[i], (void**)&status);
    assert(rc == 0);
    assert(status == 0);
  }
}

int main()
{
  atomicSum.store(0);

  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  RunThreads();

  printf("%d threads, sum: %d\n", NUM_THREADS, atomicSum.load());

#ifdef REPORT_RESULT
  int result = (atomicSum == NUM_THREADS);
  REPORT_RESULT(result);
#endif
}
