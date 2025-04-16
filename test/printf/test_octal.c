/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main() {
  printf("%o %o %o %o %o\n", 10, 256, 1023, 1001001, 12345678);
  printf("%#o %#o %#o %#o %#o\n", 10, 256, 1023, 1001001, 12345678);
  return 0;
}
