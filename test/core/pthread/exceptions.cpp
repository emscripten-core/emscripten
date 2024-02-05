// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <emscripten.h>

#include <atomic>

#define NUM_THREADS 2
#define TOTAL 1000
#define THREAD_ADDS 750
#define MAIN_ADDS 5

static std::atomic<int> sum;
static std::atomic<int> total;

void *ThreadMain(void *arg) {
  for (int i = 0; i < TOTAL; i++) {
    try {
      // Throw two different types, to make sure we check throwing and landing
      // pad behavior.
      if (i & 3) {
        throw 3.14159f;
      }
      throw i;
    } catch (int x) {
      total += x;
    } catch (float f) {
      // wait for a change, so we see interleaved processing.
      int last = ++sum;
      while (sum.load() == last) {}
    }
  }
  pthread_exit((void*)TOTAL);
}

pthread_t thread[NUM_THREADS];

void CreateThread(int i)
{
  int rc = pthread_create(&thread[i], nullptr, ThreadMain, (void*)(intptr_t)i);
  assert(rc == 0);
}

void loop() {
  static int main_adds = 0;
  int worker_adds = sum++ - main_adds++;
  printf("main iter %d : %d\n", main_adds, worker_adds);
  if (worker_adds == NUM_THREADS * THREAD_ADDS &&
      main_adds >= MAIN_ADDS) {
    printf("done: %d.\n", total.load());
    emscripten_cancel_main_loop();
    exit(0);
  }
}

int main() {
  // Create initial threads.
  for (int i = 0; i < NUM_THREADS; ++i) {
    printf("make\n");
    CreateThread(i);
  }

  emscripten_set_main_loop(loop, 0, 0);
  return 0;
}
