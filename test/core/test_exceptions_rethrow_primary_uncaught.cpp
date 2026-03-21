#include <exception>
#include <iostream>
#include <stdexcept>

struct DestructorTester {
  ~DestructorTester() {
    std::cout << "Destructor Uncaught: " << std::uncaught_exceptions() << "\n";
  }
};

int main() {
  std::exception_ptr p;
  try {
    throw std::runtime_error("test");
  } catch (...) {
    p = std::current_exception();
  }

  std::cout << "Before Uncaught: " << std::uncaught_exceptions() << "\n";
  try {
    DestructorTester dt;
    std::rethrow_exception(p);
  } catch (...) {
    std::cout << "In catch Uncaught: " << std::uncaught_exceptions() << "\n";
  }
  std::cout << "After Uncaught: " << std::uncaught_exceptions() << "\n";
  return 0;
}
