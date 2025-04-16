// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <emscripten/console.h>
#include <assert.h>

#include <thread>

struct Test {
  int threadId;
};

void *ThreadMain(void *arg) {
  emscripten_outf("Thread %d finished, exit()ing", ((Test*)arg)->threadId);
  pthread_exit(0);
}

void RunTest(int test) {
  int NUM_THREADS = std::thread::hardware_concurrency();
  assert(NUM_THREADS > 0);

  emscripten_outf("Main: Test %d starting, with num cores: %d", test, NUM_THREADS);

  struct Test t[NUM_THREADS];
  pthread_t thread[NUM_THREADS];

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 4*1024);

  emscripten_outf("Main thread has thread ID %ld", pthread_self());
  assert(pthread_self() != 0);

  emscripten_outf("Main: Starting test %d", test);

  for (int i = 0; i < NUM_THREADS; ++i) {
    t[i].threadId = i;
    int rc = pthread_create(&thread[i], &attr, ThreadMain, &t[i]);
    assert(rc == 0);
  }

  pthread_attr_destroy(&attr);

  for (int i = 0; i < NUM_THREADS; ++i) {
    int status = 1;
    int rc = pthread_join(thread[i], (void**)&status);
    assert(rc == 0);
    assert(status == 0);
  }

  emscripten_outf("Main: Test %d finished", test);
}

int main() {
  // Do a bunch of joins, verifying the Worker pool works.
  for (int i = 0; i < 7; ++i) {
    RunTest(i);
  }

  printf("Main: Test successfully finished.\n");
  return 0;
}
