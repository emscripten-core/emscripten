#include <stdio.h>
#include <threads.h>

static thread_local int foo = 10;
static thread_local int bar = 11;

void sidey();

int get_foo() {
  return foo;
}

int get_bar() {
  return bar;
}

int main(int argc, char const *argv[]) {
  printf("main TLS: %d %d\n", get_foo(), get_bar());
  sidey();
  return 0;
}
