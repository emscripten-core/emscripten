// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <atomic>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <unistd.h>

double start;

#define MAX_WORKERS 1024

int WORKERS = 0;

pthread_t thread[MAX_WORKERS] = {};

std::atomic<int> running = 0;

#define TOTAL   50000000
#define AT_ONCE     1000
#define BASE_SIZE   1000

void *ThreadMain(void *arg) {
  puts("thread started");
  running++;
  void* allocations[AT_ONCE];
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
    char* data = (char*)allocation;
    *data = i;
  }
  int total = 0;
  for (int i = 0; i < AT_ONCE; i++) {
    void* allocation = allocations[i];
    char* data = (char*)allocation;
    total += *data;
    free(allocation);
  }
  printf("thread exiting with total %d\n", total);
  running--;
  if (running == 0) {
    double end = emscripten_date_now();
    printf("total time %.2f\n", end - start);
  }
	pthread_exit(0);
}

void CreateThread(int i) {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 4*1024);
	int rc = pthread_create(&thread[i], &attr, ThreadMain, 0);
	if (rc != 0 || thread[i] == 0)
		printf("Failed to create thread!\n");
	pthread_attr_destroy(&attr);
}

int main(int argc, char **argv) {
  if (argc == 1) {
    puts("please provide the number of workers in an argument to main()");
    exit(1);
  }
  WORKERS = atoi(argv[1]);
  printf("Workers: %d\n", WORKERS);

	// Create new threads in parallel.
	for(int i = 0; i < WORKERS; ++i) {
		CreateThread(i);
	}

  start = emscripten_date_now();

	for(int i = 0; i < WORKERS; ++i) {
		void *status;
    pthread_join(thread[i], &status);
  }
}
