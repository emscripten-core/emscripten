/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

char cache[256], *next = cache;

int main() {
  cache[10] = 25;
  next[20] = 51;
  printf("*%d,%d*\n", next[10], cache[20]);
  return 0;
}
