#include <threads.h>
#include <stdio.h>
#include <threads.h>

thread_local int baz = 42;
thread_local int wiz = 43;
extern thread_local int foo;
extern thread_local int bar;

int get_foo() {
  return foo;
}

int get_bar() {
  return bar;
}

int get_baz() {
  return baz;
}

int get_wiz() {
  return wiz;
}

void sidey() {
  printf("side: foo=%d\n", get_foo());
  printf("side: bar=%d\n", get_bar());
  printf("side: baz=%d\n", get_baz());
  printf("side: wiz=%d\n", get_wiz());
}
