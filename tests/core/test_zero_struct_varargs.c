/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>

typedef struct {} zero;

typedef struct {
  int x;
  int y;
} coords;

typedef struct {
  int r;
  int g;
  int b;
} color;

void fun(int a, ...) {
  (void)a;
  va_list ap;
  va_start(ap, a);

  zero var0 = va_arg(ap, zero);
  coords var1 = va_arg(ap, coords);
  color var2 = va_arg(ap, color);
  printf("va_arg coords: %d and %d\n", var1.x, var1.y);
  printf("va_arg color: %d, %d, %d\n", var2.r, var2.g, var2.b);

  va_end(ap);
}

int main(void) {
  zero val0;
  coords val1 = { .x = 42, .y = 21 };
  color val2 = { .r = 37, .g = 19, .b = 253 };
  fun(0, val0, val1, val2);
  return 0;
}
