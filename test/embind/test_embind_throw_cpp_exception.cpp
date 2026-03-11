#include <emscripten/val.h>
#include <iostream>
using namespace emscripten;

int main() {
  val error;
  try {
    throw std::runtime_error("oopsie");
  } catch (const std::runtime_error&) {
    error = val::take_ownership(
      emscripten::internal::_emval_from_current_cxa_exception());
  }
  try {
    error.throw_();
  } catch (const std::runtime_error& ex) {
    std::clog << "Caught1: " << ex.what() << '\n';
  }

  try {
    throw std::runtime_error("oopsie");
  } catch (const std::runtime_error&) {
    error = val::take_ownership(
      emscripten::internal::_emval_from_current_cxa_exception());
  }
  try {
    throw std::runtime_error("this is bad");
  } catch (const std::runtime_error&) {
    try {
      error.throw_(); // Rethrow oopsie
    } catch (const std::runtime_error& ex) {
      std::clog << "Caught2: " << ex.what() << '\n';
    }
  }
}
