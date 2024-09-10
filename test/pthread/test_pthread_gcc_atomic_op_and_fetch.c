// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

// This file tests the old GCC built-in atomic operations of the form __sync_op_and_fetch().
// See https://gcc.gnu.org/onlinedocs/gcc-4.6.4/gcc/Atomic-Builtins.html

#define NUM_THREADS 8

#define T int

void *thread_add_and_fetch(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_add_and_fetch((int*)arg, 1);
  }
  pthread_exit(0);
}

void *thread_sub_and_fetch(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_sub_and_fetch((int*)arg, 1);
  }
  pthread_exit(0);
}

volatile long or_and_fetch_data = 0;
void *thread_or_and_fetch(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_or_and_fetch((long*)&or_and_fetch_data, (long)arg);
  }
  pthread_exit(0);
}

volatile long and_and_fetch_data = 0;
void *thread_and_and_fetch(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_and_and_fetch((long*)&and_and_fetch_data, (long)arg);
  }
  pthread_exit(0);
}

volatile long xor_and_fetch_data = 0;
void *thread_xor_and_fetch(void *arg) {
  for (int i = 0; i < 9999; ++i) // Odd number of times so that the operation doesn't cancel itself out.
    __sync_xor_and_fetch((long*)&xor_and_fetch_data, (long)arg);
  pthread_exit(0);
}

volatile long nand_and_fetch_data = 0;
void *thread_nand_and_fetch(void *arg) {
  for (int i = 0; i < 9999; ++i) { // Odd number of times so that the operation doesn't cancel itself out.
    __sync_nand_and_fetch((long*)&nand_and_fetch_data, (long)arg);
  }
  pthread_exit(0);
}

int main() {
  pthread_t thread[NUM_THREADS];

  {
    printf("__sync_add_and_fetch ..\n");
    T x = 5;
    T y = __sync_add_and_fetch(&x, 10);
    assert(y == 15);
    assert(x == 15);
    volatile int n = 1;
    if (emscripten_has_threading_support())
    {
      for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_add_and_fetch, (void*)&n);
      for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
      assert(n == NUM_THREADS*10000+1);
    }
  }
  {
    printf("__sync_sub_and_fetch ..\n");
    T x = 5;
    T y = __sync_sub_and_fetch(&x, 10);
    assert(y == -5);
    assert(x == -5);
    volatile int n = 1;
    if (emscripten_has_threading_support())
    {
      for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_sub_and_fetch, (void*)&n);
      for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
      assert(n == 1-NUM_THREADS*10000);
    }
  }
  {
    printf("__sync_or_and_fetch ..\n");
    T x = 5;
    T y = __sync_or_and_fetch(&x, 9);
    assert(y == 13);
    assert(x == 13);
    for (int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
    {
      or_and_fetch_data = (1<<NUM_THREADS);
      if (emscripten_has_threading_support())
      {
        for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_or_and_fetch, (void*)(1ll<<i));
        for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
        assert(or_and_fetch_data == (1<<(NUM_THREADS+1))-1);
      }
    }
  }
  {
    printf("__sync_and_and_fetch ..\n");
    T x = 5;
    T y = __sync_and_and_fetch(&x, 9);
    assert(y == 1);
    assert(x == 1);
    for (int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
    {
      and_and_fetch_data = (1<<(NUM_THREADS+1))-1;
      if (emscripten_has_threading_support())
      {
        for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_and_and_fetch, (void*)(~(1ll<<i)));
        for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
        assert(and_and_fetch_data == 1<<NUM_THREADS);
      }
    }
  }
  {
    printf("__sync_xor_and_fetch ..\n");
    T x = 5;
    T y = __sync_xor_and_fetch(&x, 9);
    assert(y == 12);
    assert(x == 12);
    for (int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
    {
      xor_and_fetch_data = 1<<NUM_THREADS;
      if (emscripten_has_threading_support())
      {
        for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_xor_and_fetch, (void*)(~(1ll<<i)));
        for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
        assert(xor_and_fetch_data == (1<<(NUM_THREADS+1))-1);
      }
    }
  }
  {
    printf("__sync_nand_and_fetch ..\n");
    T x = 5;
    T y = __sync_nand_and_fetch(&x, 9);
    assert(y == -2);
    assert(x == -2);
    const int oddNThreads = NUM_THREADS-1;
    for (int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
    {
      nand_and_fetch_data = 0;
      for (int i = 0; i < oddNThreads; ++i) pthread_create(&thread[i], NULL, thread_nand_and_fetch, (void*)-1);
      for (int i = 0; i < oddNThreads; ++i) pthread_join(thread[i], NULL);
      assert(nand_and_fetch_data == -1);
    }
  }

  return 0;
}
