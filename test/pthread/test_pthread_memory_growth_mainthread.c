// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten/em_asm.h>

static void *thread_start(void *arg)
{
  char* buffer = (char*)arg;
  assert(buffer);
  assert(*buffer == 42);
  EM_ASM({ assert(HEAP8[$0] === 42, "readable from JS in worker") }, buffer);
  pthread_exit((void*)43);
}

char* test() {
  // allocate more memory than we currently have, forcing a growth
  char* buffer = (char*)malloc(64 * 1024 * 1024);
  assert(buffer);
  *buffer = 42;
  return buffer;
}

int main()
{
  printf("prep\n");

  printf("start main\n");
  EM_ASM({ assert(HEAP8.length === 32 * 1024 * 1024, "start at 32MB") });
  char* buffer = test();
  assert(*buffer == 42); // should see the value the code wrote
  EM_ASM({ assert(HEAP8[$0] === 42, "readable from JS") }, buffer);
  EM_ASM({ assert(HEAP8.length > 64 * 1024 * 1024, "end with >64MB") });

  printf("start thread\n");
  pthread_t thr;
  int s = pthread_create(&thr, NULL, thread_start, (void*)buffer);
  assert(s == 0);
  printf("join\n");
  void* result = NULL;
  s = pthread_join(thr, &result);
  assert(result == (void*)43);

  printf("finish\n");

  return 0;
}

