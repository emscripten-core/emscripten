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
#include <emscripten/threading.h>

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
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(6765);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  pthread_t thr;

  printf("start\n");
  EM_ASM({ assert(HEAP8.length === 32 * 1024 * 1024, "start at 32MB") });

  printf("test\n");
  char* buffer = test();

  printf("finish\n");
  assert(*buffer == 42); // should see the value the code wrote
  EM_ASM({ assert(HEAP8.length > 64 * 1024 * 1024, "end with >64MB") });
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}

