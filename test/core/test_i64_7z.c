/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>
uint64_t a, b;
int main(int argc, char *argv[]) {
  a = argc;
  b = argv[1][0];
  printf("%d,%d\n", (int)a, (int)b);
  if (a > a + b || a > a + b + 1) {
    printf("one %lld, %lld", a, b);
    return 0;
  }
  printf("zero %lld, %lld\n", a, b);
  return 0;
}
