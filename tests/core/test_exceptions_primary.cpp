// exception_ptr example
#include <iostream>       // std::cout
#include <exception>      // std::exception_ptr, std::current_exception, std::rethrow_exception
#include <stdexcept>      // std::logic_error

int main () {
  std::exception_ptr p;
  try {
     throw std::logic_error("some logic_error exception");   // throws
  } catch(const std::exception& e) {
     p = std::current_exception();
     std::cout << "exception caught, but continuing...\n";
  }

  std::cout << "(after exception)\n";

  try {
     std::rethrow_exception (p);
  } catch (const std::exception& e) {
     std::cout << "exception caught: " << e.what() << '\n';
  }
  return 0;
}

