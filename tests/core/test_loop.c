/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
int main() {
  int x = 5;
  for (int i = 0; i < 6; i++) {
    x += x * i;
    if (x > 1000) {
      if (x % 7 == 0) printf("cheez\n");
      x /= 2;
      break;
    }
  }
  printf("*%d*\n", x);
  return 0;
}
