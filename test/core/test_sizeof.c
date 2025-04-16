// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include "emscripten.h"

struct A {
  int x, y;
};

int main(int argc, const char *argv[]) {
  int a[10];
  int b[1];
  int c[10];
  for (int i = 0; i < 10; i++) a[i] = 2;
  *b = 5;
  for (int i = 0; i < 10; i++) c[i] = 8;
  printf("*%d,%d,%d,%d,%d*\n", a[0], a[9], *b, c[0], c[9]);
  // Should overwrite a, but not touch b!
  memcpy(a, c, 10 * sizeof(int));
  printf("*%d,%d,%d,%d,%d*\n", a[0], a[9], *b, c[0], c[9]);

  // Part 2
  struct A as[3] = {{5, 12}, {6, 990}, {7, 2}};
  memcpy(&as[0], &as[2], sizeof(struct A));

  printf("*%d,%d,%d,%d,%d,%d*\n", as[0].x, as[0].y, as[1].x, as[1].y, as[2].x,
         as[2].y);

  return 0;
}
