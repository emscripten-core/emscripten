/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

struct A {
  int x;
};

struct B {
  double x;
};

void foo(int unused, ...)
{
  va_list vl;
  va_start(vl, unused);
  struct A a = va_arg(vl, struct A);
  struct B b = va_arg(vl, struct B);
  va_end(vl);

  printf("%d\n", a.x);
  printf("%f\n", b.x);
}

void a() {
  struct A a = {
    .x = 42,
  };
  struct B b = {
    .x = 42.314,
  };
  foo(0, a, b);
}

struct tiny
{
  short c;
};

void f (int n, ...)
{
  struct tiny x;
  int i;
  va_list ap;
  va_start (ap,n);
  for (i = 0; i < n; i++)
  {
    x = va_arg (ap,struct tiny);
    printf("%d : %d\n", i, x.c);
    if (x.c != i + 10) abort();
  }
  va_end (ap);
}

void b ()
{
  struct tiny x[3];
  struct tiny y;
  printf("sizeof tiny: %zu (3 of them: %zu)\n", sizeof(y), sizeof(x));
  x[0].c = 10;
  x[1].c = 11;
  x[2].c = 12;
  f (3, x[0], x[1], x[2]);
}

int main() {
  a();
  b();
  printf("ok.\n");
}

