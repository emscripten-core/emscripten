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

int main() {
  EM_ASM({
    try {
      _rethrow();
    } catch (e) {
      decrementExceptionRefcount(e);
    }
  });
}
