#include <iostream>
#include "test_whole_archive_foo.h"

extern int foo();

int main(int argc, char **argv) {
  std::cout << "Result: " << (foo() + bug::get_thing("baz")) << std::endl;
  return 0;
}

struct A {
  int i;

  A(int i) : i(i) { std::cout << "ctor a" << i << '\n'; }

  ~A() { std::cout << "dtor a" << i << '\n'; }
};

A a0(0);
