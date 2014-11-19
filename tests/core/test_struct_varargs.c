#include <stdio.h>
#include <stdarg.h>

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

int main() {
  struct A a = {
    .x = 42,
  };
  struct B b = {
    .x = 42.314,
  };
  foo(0, a, b);
  return 0;
}

