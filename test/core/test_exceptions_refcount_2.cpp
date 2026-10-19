#include <cstdio>
#include <emscripten.h>
#include <exception>

class MyException : public std::exception {
public:
  ~MyException() { puts("~MyException()"); }
};

EMSCRIPTEN_KEEPALIVE extern "C" void rethrow() {
  try {
    throw MyException();
  } catch (...) {
    throw;
  }
}

EMSCRIPTEN_KEEPALIVE extern "C" void rethrow_ptr() {
  try {
    throw MyException();
  } catch (...) {
    std::rethrow_exception(std::current_exception());
  }
}

int main() {
  EM_ASM({
    for (let fn of [_rethrow, _rethrow_ptr]) {
      try {
        fn();
      } catch (e) {
        incrementExceptionRefcount(e);
        decrementExceptionRefcount(e);
        decrementExceptionRefcount(e);
      }
    }
  });
}
