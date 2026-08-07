#include <stdexcept>
#include <stdio.h>

struct DestructorTester {
  int id;
  ~DestructorTester() {
    printf("Destructor %d Uncaught: %d\n", id, std::uncaught_exceptions());
  }
};

void foo(std::exception_ptr p) {
  DestructorTester dt1{1};
  std::rethrow_exception(p);
}

void bar(std::exception_ptr p) {
  DestructorTester dt2{2};
  foo(p);
}

int main() {
  std::exception_ptr p;
  try {
    throw std::runtime_error("test");
  } catch (...) {
    p = std::current_exception();
  }

  printf("Before Uncaught: %d\n", std::uncaught_exceptions());
  try {
    bar(p);
  } catch (...) {
    printf("In catch Uncaught: %d\n", std::uncaught_exceptions());
  }
  printf("After Uncaught: %d\n", std::uncaught_exceptions());
  return 0;
}
