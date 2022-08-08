/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

static void cleanA() { printf("A\n"); }
static void cleanB() { printf("B\n"); }

int main() {
  atexit(cleanA);
  atexit(cleanB);
  return 0;
}
