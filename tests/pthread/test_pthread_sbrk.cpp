// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define NUM_THREADS 8
#define NUM_ALLOCATIONS 10240
#if ABORTING_MALLOC
#define ALLOCATION_SIZE 1280 // Malloc aborts, so allocate a bit less of memory so all fits
#else
#define ALLOCATION_SIZE 2560 // Malloc doesn't abort, allocate a bit more memory to test graceful allocation failures
#endif

#define RESULT_OK 0
#define RESULT_EXPECTED_FAILS 1
#define RESULT_BAD_FAIL 2

// Use barriers to make each thread synchronize their execution points, to maximize the possibility of seeing race conditions
// if those might occur.
static pthread_barrier_t barrierWaitToAlloc;
static pthread_barrier_t barrierWaitToVerify;
static pthread_barrier_t barrierWaitToFree;

// Use a mutex for logging.
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *thread_start(void *arg)
{
#if DEBUG
  pthread_mutex_lock( &mutex );
  printf("thread started, will try %d allocations of size %d\n", NUM_ALLOCATIONS, ALLOCATION_SIZE);
  pthread_mutex_unlock( &mutex );
#endif

  int id = (int)(arg)+1;
  int return_code = RESULT_OK;

  uint8_t *allocated_buffers[NUM_ALLOCATIONS] = {};

  int some_allocations_failed = 0;
  size_t allocated = 0;

  pthread_barrier_wait(&barrierWaitToAlloc); // Halt until all threads reach here, then proceed synchronously.
  for(int i = 0; i < NUM_ALLOCATIONS; ++i)
  {
    allocated_buffers[i] = (uint8_t*)malloc(ALLOCATION_SIZE);
    if (allocated_buffers[i]) {
      memset(allocated_buffers[i], id, ALLOCATION_SIZE);
      allocated += ALLOCATION_SIZE;
    } else
      some_allocations_failed = 1;
  }
#if DEBUG
  pthread_mutex_lock( &mutex );
  printf("total allocations: %u (%d of size %d tried), some failed? %d\n", allocated, NUM_ALLOCATIONS, ALLOCATION_SIZE, some_allocations_failed);
  pthread_mutex_unlock( &mutex );
#endif
  pthread_barrier_wait(&barrierWaitToVerify); // Halt until all threads reach here, then proceed synchronously.
  int reported_once = 0;
  for(int i = 0; i < NUM_ALLOCATIONS; ++i)
  {
    if (!allocated_buffers[i]) continue;
    for(int j = 0; j < ALLOCATION_SIZE; ++j)
      if (allocated_buffers[i][j] != id)
      {
        ++return_code; // Failed! (but run to completion so that the barriers will all properly proceed without hanging)
        if (!reported_once) {
          EM_ASM(console.error('Memory corrupted! mem[i]: ' + $0 + ' != ' + $1 + ', i: ' + $2 + ', j: ' + $3), allocated_buffers[i][j], id, i, j);
          reported_once = 1; // Avoid print flood that makes debugging hard.
        }
      }
  }

  pthread_barrier_wait(&barrierWaitToFree); // Halt until all threads reach here, then proceed synchronously.
  for(int i = 0; i < NUM_ALLOCATIONS; ++i)
    free(allocated_buffers[i]);

#if ABORTING_MALLOC
  if (some_allocations_failed)
    return_code = RESULT_BAD_FAIL; // We expect allocations not to fail (if they did, shouldn't reach here, but we should have aborted)
#else
  if (some_allocations_failed)
    return_code = RESULT_EXPECTED_FAILS; // We expect to be allocating so much memory that some of the allocations fail.
  // Otherwise, the fails might happen in another thread, that's cool.
#endif
#if DEBUG
  pthread_mutex_lock( &mutex );
  printf("the pthread return code: %d\n", return_code);
  pthread_mutex_unlock( &mutex );
#endif
  pthread_exit((void*)return_code);
}

int main()
{
  int result = 0;
  if (!emscripten_has_threading_support()) {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: threading support is not available!\n");
    return 0;
  }

  printf("starting test, aborting? %d\n", ABORTING_MALLOC);

  int ret = pthread_barrier_init(&barrierWaitToAlloc, NULL, NUM_THREADS);
  assert(ret == 0);
  ret = pthread_barrier_init(&barrierWaitToVerify, NULL, NUM_THREADS);
  assert(ret == 0);
  ret = pthread_barrier_init(&barrierWaitToFree, NULL, NUM_THREADS);
  assert(ret == 0);

  pthread_t thr[8/*NUM_THREADS*/];
  for(int i = 0; i < NUM_THREADS; ++i)
  {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, NUM_ALLOCATIONS*80);
    ret = pthread_create(&thr[i], &attr, thread_start, (void*)(i));
    assert(ret == 0);
  }

  int seen_expected_fails = 0;

  for(int i = 0; i < NUM_THREADS; ++i) {
    int res = 0;
    ret = pthread_join(thr[i], (void**)&res);
    assert(ret == 0);
    if (res == RESULT_OK) {
    } else if (res == RESULT_EXPECTED_FAILS) {
      seen_expected_fails = 1;
    } else if (res == RESULT_BAD_FAIL) {
      result = 1;
    }
    if (res) printf("Thread %d failed with return code %d.\n", i, res);
  }
#if !ABORTING_MALLOC
  if (!seen_expected_fails) {
    printf("Expected to see fails, but saw none :(\n");
    result = 2;
  }
#endif

  printf("Test finished with result %d\n", result);

#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}
