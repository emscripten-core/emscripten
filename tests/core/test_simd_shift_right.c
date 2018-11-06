/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

typedef int int32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned int uint32x4 __attribute__((__vector_size__(16), __may_alias__));

int main() {

  uint32x4 v = { 0, 0x80000000u, 0x7fffffff, 0xffffffffu};
  v = v >> 2;
  printf("%x %x %x %x\n", v[0], v[1], v[2], v[3]);

  int32x4 w = { 0, 0x80000000u, 0x7fffffff, 0xffffffffu};
  w = w >> 2;
  printf("%x %x %x %x\n", w[0], w[1], w[2], w[3]);
}
