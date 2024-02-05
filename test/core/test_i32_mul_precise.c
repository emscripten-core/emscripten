/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
  uint32_t d1 = 0x847c9b5d;
  uint32_t q = 0x549530e1;
  if (argc > 1000) {
    q += argc;
    d1 -= argc;
  }  // confuse optimizer
  printf("%u\n", d1 * q);
  return 0;
}
