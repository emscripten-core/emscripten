#include <emscripten/val.h>
#include <exception>
#include <iostream>

using namespace emscripten;

int main() {
  val error = val::null();
  try {
    throw std::runtime_error("test exception");
  } catch (const std::exception& e) {
    // Capture the exception
    error = val::take_ownership(
      emscripten::internal::_emval_from_current_cxa_exception());
  }

  std::cout << "Captured exception." << std::endl;

  int uncaught_before = std::uncaught_exceptions();
  std::cout << "Uncaught before throw 1: " << uncaught_before << std::endl;

  // First throw
  try {
    std::cout << "Throwing 1..." << std::endl;
    error.throw_();
  } catch (const std::exception& e) {
    std::cout << "Caught 1: " << e.what() << std::endl;
    int uncaught_during = std::uncaught_exceptions();
    std::cout << "Uncaught during catch 1: " << uncaught_during << std::endl;
  }

  int uncaught_between = std::uncaught_exceptions();
  std::cout << "Uncaught between throws: " << uncaught_between << std::endl;

  // Second throw - if refcount was messed up, this might fail/crash
  try {
    std::cout << "Throwing 2..." << std::endl;
    error.throw_();
  } catch (const std::exception& e) {
    std::cout << "Caught 2: " << e.what() << std::endl;
    int uncaught_during = std::uncaught_exceptions();
    std::cout << "Uncaught during catch 2: " << uncaught_during << std::endl;
  }

  std::cout << "Done." << std::endl;
  return 0;
}
