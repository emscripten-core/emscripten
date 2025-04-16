/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
struct bitty {
  unsigned x : 1;
  unsigned y : 1;
  unsigned z : 1;
};
int main() {
  struct bitty b;
  printf("*");
  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++)
      for (int k = 0; k <= 1; k++) {
        b.x = i;
        b.y = j;
        b.z = k;
        printf("%d,%d,%d,", b.x, b.y, b.z);
      }
  printf("*\n");
  return 0;
}
