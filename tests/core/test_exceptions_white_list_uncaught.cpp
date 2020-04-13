// Taken from https://en.cppreference.com/w/cpp/error/uncaught_exception and
// modified

#include <exception>
#include <iostream>
#include <stdexcept>

struct Foo {
  int count = std::uncaught_exceptions();
  ~Foo() {
    std::cout << (count == std::uncaught_exceptions()
                      ? "~Foo() called normally\n"
                      : "~Foo() called during stack unwinding\n");
  }
};

void test() {
  Foo f;
  try {
    Foo f;
    std::cout << "Exception thrown\n";
    throw std::runtime_error("test exception");
  } catch (const std::exception &e) {
    std::cout << "Exception caught: " << e.what() << '\n';
  }
}

int main() {
  test();
  return 0;
}
