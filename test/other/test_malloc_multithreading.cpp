// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <atomic>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

double start;

// You should define WORKERS, which must be less than the max.
#define MAX_WORKERS 1024

pthread_t thread[MAX_WORKERS] = {};

std::atomic<int> running = 0;

#ifndef TOTAL
#define TOTAL  100000000
#endif

#define AT_ONCE     1024
#define BASE_SIZE   1000

void *ThreadMain(void *arg) {
  puts("thread started");
  running++;
  void** allocations = (void**)malloc(AT_ONCE * sizeof(void*));
  for (int i = 0; i < AT_ONCE; i++) {
    allocations[i] = NULL;
  }
  for (int i = 0; i < (TOTAL / WORKERS); i++) {
    int rem = i & (AT_ONCE - 1);
    void*& allocation = allocations[rem];
    if (allocation) {
      free(allocation);
    }
    allocation = malloc(BASE_SIZE + rem);
    if (!allocation) {
      puts("failed to allocate");
      abort();
    }
    char* data = (char*)allocation;
    *data = i;
  }
  int total = 0;
  for (int i = 0; i < AT_ONCE; i++) {
    void* allocation = allocations[i];
    if (!allocation) {
      puts("did not have allocation");
      abort();
    }
    char* data = (char*)allocation;
    total += *data;
    free(allocation);
  }
  free(allocations);
  printf("thread exiting with total %d\n", total);
  running--;
  if (running == 0) {
#if VERBOSE
    double end = emscripten_date_now();
    printf("total time %.2f\n", end - start);
#endif
    printf("Done.\n");
  }
  pthread_exit(0);
}

void CreateThread(int i) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 4*1024);
  int rc = pthread_create(&thread[i], &attr, ThreadMain, 0);
  if (rc != 0 || thread[i] == 0) {
    printf("Failed to create thread!\n");
  }
  pthread_attr_destroy(&attr);
}

int main() {
  printf("Workers: %d\n", WORKERS);
  assert(WORKERS < MAX_WORKERS);

#if WORKERS > 1
  // Create new threads.
  for (int i = 0; i < WORKERS; ++i) {
    CreateThread(i);
  }
#endif

#if VERBOSE
  start = emscripten_date_now();
#endif

#if WORKERS > 1
  // Join the threads as they complete their work.
  for (int i = 0; i < WORKERS; ++i) {
    void *status;
    pthread_join(thread[i], &status);
  }
#else
  // One worker, just run it.
  ThreadMain(NULL);
#endif
}
