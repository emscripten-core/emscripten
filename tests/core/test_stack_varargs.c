/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// We should not blow up the stack with numerous varargs
#include <stdio.h>
#include <stdlib.h>

void func(int i) {
  printf(
      "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
      "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
      i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i,
      i, i, i, i, i, i, i, i, i, i, i, i, i, i);
}
int main() {
  for (int i = 0; i < 4 * 1024; i++) func(i);
  printf("ok!\n");
  return 0;
}
