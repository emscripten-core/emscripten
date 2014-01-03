#include <stdio.h>
#include <assert.h>

int foo();
int foo2();
int foo3();
int foo4();
int foo5();
int foo6();
int foo7();
int foo8();
int foo9();
int foo10();

int main() {
  assert(foo() == 1);
  assert(foo2() == 1);
  assert(foo3() == 1);
  assert(foo4() == 1);
  assert(foo5() == 1);
  assert(foo6() == 1);
  assert(foo7() == 1);
  assert(foo8() == 1);
  assert(foo9() == 1);
  assert(foo10() == 1);
  printf("success!\n");
  return 1;
}
