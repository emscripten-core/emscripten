/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
struct {
  int y[10];
  int z[10];
} commonblock;

int main() {
  for (int i = 0; i < 10; ++i) {
    commonblock.y[i] = 1;
    commonblock.z[i] = 2;
  }
  printf("*%d %d*\n", commonblock.y[0], commonblock.z[0]);
  return 0;
}
