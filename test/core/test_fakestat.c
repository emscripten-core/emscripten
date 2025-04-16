/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
struct stat {
  int x, y;
};
int main() {
  struct stat s;
  s.x = 10;
  s.y = 22;
  printf("*%d,%d*\n", s.x, s.y);
}
