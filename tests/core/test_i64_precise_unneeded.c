/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <inttypes.h>
#include <stdio.h>

int main(int argc, char **argv) {
  uint64_t x = 2125299906845564, y = 1225891506842664;
  if (argc == 12) {
    x = x >> 1;
    y = y >> 1;
  }
  x = x & 12ULL;
  y = y | 12ULL;
  x = x ^ y;
  x <<= 2;
  y >>= 3;
  printf("*%llu, %llu*\n", x, y);
}
