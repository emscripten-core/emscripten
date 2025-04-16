/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "emscripten.h"

int main(int argc, char **argv)
{
  char *buf1 = malloc(100);
  char *data1 = "hello";
  memcpy(buf1, data1, strlen(data1)+1);

  float *buf2 = malloc(100);
  float pie = 4.955;
  memcpy(buf2, &pie, sizeof(float));

  printf("*pre: %s,%.3f*\n", buf1, buf2[0]);

  size_t totalMemory = (size_t)EM_ASM_PTR({ return HEAP8.length });
  //printf("totalMemory: %zu, argc: %d\n", totalMemory, argc);
  char *buf3 = malloc(totalMemory+1);
  assert(buf3 && "fail to perform large allocation");
  if (argc % 7 == 6) printf("%p\n", memcpy(buf3, buf1, argc));
  char *buf4 = malloc(100);
  float *buf5 = malloc(100);
  //printf("totalMemory: %d bufs: %d,%d,%d,%d,%d\n", totalMemory, buf1, buf2, buf3, buf4, buf5);
  assert((intptr_t)buf4 > (intptr_t)totalMemory && (intptr_t)buf5 > (intptr_t)totalMemory);

  printf("*%s,%.3f*\n", buf1, buf2[0]); // the old heap data should still be there

  memcpy(buf4, buf1, strlen(data1)+1);
  memcpy(buf5, buf2, sizeof(float));
  printf("*%s,%.3f*\n", buf4, buf5[0]); // and the new heap space should work too

  return 0;
}
