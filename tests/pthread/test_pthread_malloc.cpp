// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 8
#define N 6

static void *thread_start(void *arg)
{
  long n = (long)arg;
  long *mem[N] = {};
  for(long i = 0; i < N; ++i)
  {
    mem[i] = (long*)malloc(4);
    *mem[i] = n+i;
  }
  for(long i = 0; i < N; ++i)
  {
    long k = *mem[i];
    if (k != n+i)
    {
      EM_ASM(console.error('Memory corrupted! mem[i]: ' + $0 + ', i: ' + $1 + ', n: ' + $2), k, i, n);
      pthread_exit((void*)1);
    }

    assert(*mem[i] == n+i);
    free(mem[i]);
  }
  EM_ASM(console.log('Worker with task number ' + $0 + ' finished.'), n);
  pthread_exit(0);
}

int main()
{
  if (!emscripten_has_threading_support()) {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: threading support is not available!\n");
    return 0;
  }

  pthread_t thr[NUM_THREADS];
  for(int i = 0; i < NUM_THREADS; ++i)
    pthread_create(&thr[i], NULL, thread_start, (void*)(i*N));
  int result = 0;
  for(int i = 0; i < NUM_THREADS; ++i) {
    int res = 0;
    pthread_join(thr[i], (void**)&res);
    result += res;
  }
  printf("Test finished with result %d\n", result);

#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}
