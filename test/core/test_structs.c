/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

struct S {
  int x, y;
};

int main() {
  struct S a, b;
  a.x = 5;
  a.y = 6;
  b.x = 101;
  b.y = 7009;
  struct S *c, *d;
  c = &a;
  c->x *= 2;
  c = &b;
  c->y -= 1;
  d = c;
  d->y += 10;
  printf("*%d,%d,%d,%d,%d,%d,%d,%d*\n", a.x, a.y, b.x, b.y, c->x, c->y, d->x,
         d->y);
  return 0;
}
