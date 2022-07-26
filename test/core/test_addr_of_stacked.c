/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
void alter(int *y) { *y += 5; }
int main() {
  int x = 2;
  alter(&x);
  printf("*%d*\n", x);
  return 0;
}
