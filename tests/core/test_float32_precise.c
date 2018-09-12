/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main(int argc, char **argv) {
  float x = 1.23456789123456789;
  float y = 5.20456089123406709;
  while (argc > 10 || argc % 19 == 15) {
    // confuse optimizer
    x /= y;
    y = 2 * y - 1;
    argc--;
  }
  x = x - y;
  y = 3 * y - x / 2;
  x = x * y;
  y += 0.000000000123123123123;
  x -= y / 7.654;
  printf("\n%.20f, %.20f\n", x, y);
  return 0;
}
