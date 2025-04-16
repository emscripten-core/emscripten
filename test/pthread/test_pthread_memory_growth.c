// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>

static void *thread_start(void *arg)
{
  // allocate more memory than we currently have, forcing a growth
  printf("thread_start\n");
  char* buffer = (char*)malloc(64 * 1024 * 1024);
  assert(buffer);
  *buffer = 42;
  pthread_exit((void*)buffer);
}

int main()
{
  printf("prep\n");

  pthread_t thr;

  printf("start\n");
  EM_ASM({ assert(HEAP8.length === 32 * 1024 * 1024, "start at 32MB") });

  printf("create\n");
  int s = pthread_create(&thr, NULL, thread_start, (void*)NULL);
  assert(s == 0);
  void* result = NULL;

  printf("join\n");
  s = pthread_join(thr, &result);
  assert(result != 0); // allocation should have succeeded
  char* buffer = (char*)result;
  assert(*buffer == 42); // should see the value the thread wrote
  EM_ASM({ assert(HEAP8.length > 64 * 1024 * 1024, "end with >64MB") });
  return 0;
}

