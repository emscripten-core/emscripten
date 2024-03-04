// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <assert.h>

#define NUM_THREADS 8
#define DATA_COUNT 10

volatile double globalDouble = 0.0;
volatile uint64_t globalU64 = 0;

uint64_t sharedData[DATA_COUNT] = {};

struct Test {
  int op;
  int threadId;
};

uint64_t threadCasAccumulatedWrittenData[NUM_THREADS] = {};
uint64_t threadCasAccumulatedReadData[NUM_THREADS] = {};

int64_t rand_60() {
  return (int64_t)(emscripten_random() * (float)0x3FFFFFFF) | ((int64_t)(emscripten_random() * (float)0x3FFFFFFF) << 30);
}

void *ThreadMain(void *arg) {
  assert(pthread_self() != 0);
  assert(globalDouble == 5.0);
  assert(globalU64 == 5);
  struct Test *t = (struct Test*)arg;
  emscripten_outf("Thread %d for test %d: starting computation", t->threadId, t->op);

  for (int i = 0; i < 99999; ++i) {
    for (int j = 0; j < DATA_COUNT; ++j) {
      switch (t->op) {
        case 0: emscripten_atomic_add_u64(&sharedData[j], 1); break;
        case 1: emscripten_atomic_sub_u64(&sharedData[j], 1); break;
        case 2: emscripten_atomic_and_u64(&sharedData[j], ~(1UL << t->threadId)); break;
        case 3: emscripten_atomic_or_u64(&sharedData[j], 1UL << t->threadId); break;
        case 4: emscripten_atomic_xor_u64(&sharedData[j], 1UL << t->threadId); break;
        case 5: {
          // Atomically load and store data, and test that each individual u8 is the same.
          uint64_t data = emscripten_atomic_load_u64(&sharedData[j]);
          uint8_t dataU8[8];
          memcpy(dataU8, &data, 8);
          assert(dataU8[0] >= 10 && dataU8[0] < 10+NUM_THREADS);
          assert(dataU8[0] == dataU8[1] && dataU8[0] == dataU8[2] && dataU8[0] == dataU8[3]);
          assert(dataU8[0] == dataU8[4] && dataU8[0] == dataU8[5] && dataU8[0] == dataU8[6] && dataU8[0] == dataU8[7]);
          dataU8[0] = dataU8[1] = dataU8[2] = dataU8[3] = dataU8[4] = dataU8[5] = dataU8[6] = dataU8[7] = 10 + t->threadId;
          memcpy(&data, dataU8, 8);
          emscripten_atomic_store_u64(&sharedData[j], data);
        }
        break;
        case 6: {
          uint64_t newData = rand_60();
          uint64_t data;
          uint64_t prevData;
          do {
            data = emscripten_atomic_load_u64(&sharedData[j]);
            prevData = emscripten_atomic_cas_u64(&sharedData[j], data, newData);
          } while(prevData != data);
          threadCasAccumulatedReadData[t->threadId] += data;
          threadCasAccumulatedWrittenData[t->threadId] += newData;
        }
        break;
      }
    }
  }
  emscripten_outf("Thread %d for test %d: finished, exit()ing", t->threadId, t->op);
  pthread_exit(0);
}

struct Test t[NUM_THREADS] = {};
pthread_t thread[NUM_THREADS];

void RunTest(int test) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 4*1024);

  emscripten_outf("Main thread has thread ID %p\n", pthread_self());
  assert(pthread_self() != 0);

  switch(test) {
    case 2: memset(sharedData, 0xFF, sizeof(sharedData)); break;
    case 5: memset(sharedData, 0x10, sizeof(sharedData)); break;
    default: memset(sharedData, 0, sizeof(sharedData)); break;
  }

  emscripten_outf("Main: Starting test %d", test);

  for (int i = 0; i < NUM_THREADS; ++i) {
    t[i].op = test;
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

  int val = sharedData[0];
  emscripten_outf("Main: Test %d finished. Result: %d", test, val);
  if (test != 6) {
    for (int i = 1; i < DATA_COUNT; ++i) {
      assert(sharedData[i] == sharedData[0]);
    }
  }
}

int main() {
  globalDouble = 5.0;
  globalU64 = 4;

  uint64_t prevU64 = emscripten_atomic_add_u64((void*)&globalU64, 1);
  assert(prevU64 == 4);

  for (int i = 0; i < 7; ++i) {
    RunTest(i);
  }

  uint64_t totalRead = 0;
  uint64_t totalWritten = 0;
  for (int i = 0; i < NUM_THREADS; ++i) {
    totalRead += threadCasAccumulatedReadData[i];
    totalWritten += threadCasAccumulatedWrittenData[i];
  }
  for (int i = 0; i < DATA_COUNT; ++i) {
    totalRead += sharedData[i];
  }

  if (totalRead == totalWritten) {
    emscripten_outf("totalRead: %llu, totalWritten: %llu\n", totalRead, totalWritten);
  } else {
    emscripten_outf("64-bit CAS test failed! totalRead != totalWritten (%llu != %llu)\n", totalRead, totalWritten);
  }
  assert(totalRead == totalWritten);
  emscripten_outf("Main: Test successfully finished");
  return 0;
}
