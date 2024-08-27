/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int test(unsigned short a, unsigned short b) {
  unsigned short result = a;
  result += b;
  if (result < b) printf("C!");
  return result;
}

int main(void) {
  printf(",%d,", test(0, 0));
  printf(",%d,", test(1, 1));
  printf(",%d,", test(65535, 1));
  printf(",%d,", test(1, 65535));
  printf(",%d,", test(32768, 32767));
  printf(",%d,", test(32768, 32768));
  printf("\n");
  return 0;
}
