/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>

typedef struct {
  int x;
  int y;
} coords;

void fun(int a, ...) {
  (void)a;
  va_list ap;
  va_start(ap, a);

  coords var = va_arg(ap, coords);
  printf("va_arg struct: %d and %d\n", var.x, var.y);

  va_end(ap);
}

int main(void) {
  coords val = { .x=42, .y=21 };
  printf("value passed struct: %d and %d\n", val.x, val.y);
  fun(0, val);
  return 0;
}
