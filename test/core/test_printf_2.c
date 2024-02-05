/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>

int main() {
  char c = '1';
  short s = 2;
  int i = 3;
  long long l = 4;
  float f = 5.5;
  double d = 6.6;

  printf("%c,%hd,%d,%lld,%.1f,%.1f\n", c, s, i, l, f, d);
  printf("%#x,%#x\n", 1, 0);

  // %m is a gnu extension which is equivalent to strerror(errnor).
  // It is implemented in musl but we disable it under emscripten because it
  // pulls in extra code.
  assert(printf("%m\n") < 0);

  printf("done\n");
  return 0;
}
