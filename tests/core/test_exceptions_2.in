#include <stdexcept>
#include <stdio.h>

typedef void (*FuncPtr)();

void ThrowException() { throw std::runtime_error("catch me!"); }

FuncPtr ptr = ThrowException;

int main() {
  try {
    ptr();
  }
  catch (...) {
    printf("Exception caught successfully!\n");
  }
  return 0;
}
