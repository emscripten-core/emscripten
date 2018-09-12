/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdint.h>

int main()
{
  // i32 vs i64
  int32_t small = -1;
  int64_t large = -1;
  printf("*%d*\n", small == large);
  small++;
  printf("*%d*\n", small == large);
  uint32_t usmall = -1;
  uint64_t ularge = -1;
  printf("*%d*\n", usmall == ularge);
  return 0;
}
