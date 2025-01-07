/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
int main(int argc, char* argv[]) {
  int one = argc;

  printf("%d ", 0 * one);
  printf("%d ", 0 * -one);
  printf("%d ", -one * 0);
  printf("%g ", 0.0 * one);
  printf("%g ", 0.0 * -one);
  printf("%g\n", -one * 0.0);
  return 0;
}
