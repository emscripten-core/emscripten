// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

// This file tests the old GCC built-in atomic operations of the form __sync_fetch_and_op().
// See https://gcc.gnu.org/onlinedocs/gcc-4.6.4/gcc/Atomic-Builtins.html

#define NUM_THREADS 8

#define T int

void *thread_fetch_and_add(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_fetch_and_add((int*)arg, 1);
  }
  pthread_exit(0);
}

void *thread_fetch_and_sub(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_fetch_and_sub((int*)arg, 1);
  }
  pthread_exit(0);
}

volatile long fetch_and_or_data = 0;
void *thread_fetch_and_or(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_fetch_and_or(&fetch_and_or_data, (long)arg);
  }
  pthread_exit(0);
}

volatile long fetch_and_and_data = 0;
void *thread_fetch_and_and(void *arg) {
  for (int i = 0; i < 10000; ++i) {
    __sync_fetch_and_and(&fetch_and_and_data, (long)arg);
  }
  pthread_exit(0);
}

volatile long fetch_and_xor_data = 0;
void *thread_fetch_and_xor(void *arg) {
  for (int i = 0; i < 9999; ++i) { // Odd number of times so that the operation doesn't cancel itself out.
    __sync_fetch_and_xor(&fetch_and_xor_data, (long)arg);
  }
  pthread_exit(0);
}

// XXX NAND support does not exist in Atomics API.
volatile long fetch_and_nand_data = 0;
void *thread_fetch_and_nand(void *arg) {
  for (int i = 0; i < 9999; ++i) { // Odd number of times so that the operation doesn't cancel itself out.
    __sync_fetch_and_nand(&fetch_and_nand_data, (long)arg);
  }
  pthread_exit(0);
}

int main() {
  pthread_t thread[NUM_THREADS];

  {
    printf("__sync_fetch_and_add ..\n");
    T x = 5;
    T y = __sync_fetch_and_add(&x, 10);
    assert(y == 5);
    assert(x == 15);
    volatile int n = 1;
    if (emscripten_has_threading_support()) {
      for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_fetch_and_add, (void*)&n);
      for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
      assert(n == NUM_THREADS*10000+1);
    }
  }
  {
    printf("__sync_fetch_and_sub ..\n");
    T x = 5;
    T y = __sync_fetch_and_sub(&x, 10);
    assert(y == 5);
    assert(x == -5);
    volatile int n = 1;
    if (emscripten_has_threading_support()) {
      for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_fetch_and_sub, (void*)&n);
      for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
      assert(n == 1-NUM_THREADS*10000);
    }
  }
  {
    printf("__sync_fetch_and_or ..\n");
    T x = 5;
    T y = __sync_fetch_and_or(&x, 9);
    assert(y == 5);
    assert(x == 13);
    for (int x = 0; x < 100; ++x) { // Test a few times for robustness, since this test is so short-lived.
      fetch_and_or_data = (1<<NUM_THREADS);
      if (emscripten_has_threading_support()) {
        for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_fetch_and_or, (void*)(1ll<<i));
        for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
        assert(fetch_and_or_data == (1<<(NUM_THREADS+1))-1);
      }
    }
  }
  {
    printf("__sync_fetch_and_and ..\n");
    T x = 5;
    T y = __sync_fetch_and_and(&x, 9);
    assert(y == 5);
    assert(x == 1);
    for (int x = 0; x < 100; ++x) { // Test a few times for robustness, since this test is so short-lived.
      fetch_and_and_data = (1<<(NUM_THREADS+1))-1;
      if (emscripten_has_threading_support()) {
        for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_fetch_and_and, (void*)(~(1ll<<i)));
        for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
        assert(fetch_and_and_data == 1<<NUM_THREADS);
      }
    }
  }
  {
    printf("__sync_fetch_and_xor ..\n");
    T x = 5;
    T y = __sync_fetch_and_xor(&x, 9);
    assert(y == 5);
    assert(x == 12);
    for (int x = 0; x < 100; ++x) { // Test a few times for robustness, since this test is so short-lived.
      fetch_and_xor_data = 1<<NUM_THREADS;
      if (emscripten_has_threading_support()) {
        for (int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_fetch_and_xor, (void*)(~(1ll<<i)));
        for (int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
        assert(fetch_and_xor_data == (1<<(NUM_THREADS+1))-1);
      }
    }
  }

  {
    printf("__sync_fetch_and_nand ..\n");
    T x = 5;
    T y = __sync_fetch_and_nand(&x, 9);
    assert(y == 5);
    assert(x == -2);
    const int oddNThreads = NUM_THREADS-1;
    for (int x = 0; x < 100; ++x) { // Test a few times for robustness, since this test is so short-lived.
      fetch_and_nand_data = 0;
      for (int i = 0; i < oddNThreads; ++i) pthread_create(&thread[i], NULL, thread_fetch_and_nand, (void*)-1);
      for (int i = 0; i < oddNThreads; ++i) pthread_join(thread[i], NULL);
      assert(fetch_and_nand_data == -1);
    }
  }

  // Test that regex replacing also works on these.
  emscripten_atomic_fence();
  __sync_synchronize();

  return 0;
}
