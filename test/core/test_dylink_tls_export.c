#include <threads.h>
#include <stdio.h>

thread_local int foo = 10;
thread_local int bar = 11;
extern thread_local int baz;
extern thread_local int wiz;

void sidey();

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

int main(int argc, char const *argv[]) {
  printf("main: foo=%d\n", get_foo());
  printf("main: bar=%d\n", get_bar());
  printf("main: baz=%d\n", get_baz());
  printf("main: wiz=%d\n", get_wiz());
  sidey();
  return 0;
}
