// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten/console.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 8
#define N 6

static void *thread_start(void *arg) {
  long n = (long)arg;
  long *mem[N] = {};
  for (long i = 0; i < N; ++i) {
    mem[i] = (long*)malloc(4);
    *mem[i] = n+i;
  }
  for (long i = 0; i < N; ++i) {
    long k = *mem[i];
    if (k != n+i) {
      emscripten_errf("Memory corrupted! mem[i]: %ld, i: %ld, n: %ld", k, i, n);
      pthread_exit((void*)1);
    }

    assert(*mem[i] == n+i);
    free(mem[i]);
  }
  emscripten_outf("Worker with task number %ld finished", n);
  pthread_exit(0);
}

int main() {
  pthread_t thr[NUM_THREADS];
  for (intptr_t i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&thr[i], NULL, thread_start, (void*)(i*N));
  }
  int result = 0;
  for (int i = 0; i < NUM_THREADS; ++i) {
    int res = 0;
    pthread_join(thr[i], (void**)&res);
    result += res;
  }
  printf("Test finished with result %d\n", result);
  assert(result == 0);
  return 0;
}
