// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <vector>

#define NUM_BLOCKS_TO_ALLOC 50000
#define NUM_THREADS 8

pthread_mutex_t vectorMutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<void*> allocatedMemory;

static void *thread_start(void *arg) {
  for (int i = 0; i < NUM_BLOCKS_TO_ALLOC; ++i) {
    void *mem = malloc(4);
    pthread_mutex_lock(&vectorMutex);
    allocatedMemory.push_back(mem);
    pthread_mutex_unlock(&vectorMutex);
  }
  pthread_exit(0);
}

int main() {
  pthread_t thr[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_create(&thr[i], NULL, thread_start, 0);
    assert(rc == 0);
  }
  unsigned long numBlocksToFree = NUM_BLOCKS_TO_ALLOC * NUM_THREADS;
  while (numBlocksToFree > 0) {
    pthread_mutex_lock(&vectorMutex);
    for (size_t i = 0; i < allocatedMemory.size(); ++i) {
      free(allocatedMemory[i]);
    }
    numBlocksToFree -= allocatedMemory.size();
    allocatedMemory.clear();
    pthread_mutex_unlock(&vectorMutex);
  }
  for (int i = 0; i < NUM_THREADS; ++i) {
    int res = 0;
    int rc = pthread_join(thr[i], (void**)&res);
    assert(rc == 0);
    assert(res == 0);
  }
  printf("Test finished successfully!\n");
  return 0;
}
