// Must be a standard C++ header to implicitly test CMake not emitting -isystem for some imported targets: see #17132
#include <iostream>
#include <foo.h>

int main() {
  std::cout << "foo: " << foo() << std::endl;
  return 0;
}
