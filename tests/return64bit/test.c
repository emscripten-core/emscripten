/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

long long test_return64(long long input) {
  long long x = ((long long)1234 << 32) + 5678;
  printf("input = 0x%llx\n", input);
  return x;
}
