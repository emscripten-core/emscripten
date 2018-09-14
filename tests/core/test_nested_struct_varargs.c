/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>

struct A {
  int x;
};

struct B {
  double x;
};

struct C {
  char c[9];
  struct A a;
  struct B b;
};

void foo(int unused, ...)
{
  va_list vl;
  va_start(vl, unused);
  struct C c = va_arg(vl, struct C);
  va_end(vl);

  printf("%d\n", c.a.x);
  printf("%f\n", c.b.x);
  printf("%s\n", c.c);
}

int main() {
  struct A a = {
    .x = 42,
  };
  struct B b = {
    .x = 42.314,
  };
  struct C c = {
    .a = a,
    .b = b,
    .c = "nicetest",
  };
  foo(0, c);
  return 0;
}

