#include <stdexcept>
#include <stdio.h>

struct DestructorTester {
  ~DestructorTester() {
    printf("Destructor Uncaught: %d\n", std::uncaught_exceptions());
  }
};

int main() {
  std::exception_ptr p;
  try {
    throw std::runtime_error("test");
  } catch (...) {
    p = std::current_exception();
  }

  printf("Before Uncaught: %d\n", std::uncaught_exceptions());
  try {
    DestructorTester dt;
    std::rethrow_exception(p);
  } catch (...) {
    printf("In catch Uncaught: %d\n", std::uncaught_exceptions());
  }
  printf("After Uncaught: %d\n", std::uncaught_exceptions());
  return 0;
}
