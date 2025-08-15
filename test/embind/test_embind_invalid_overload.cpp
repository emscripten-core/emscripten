#include <functional>
#include <emscripten/bind.h>

using namespace emscripten;

int main() {
}

int foo(int a) {
  return 0;
}

int foo(float a) {
  return 0;
}

EMSCRIPTEN_BINDINGS(bindings) {
  // Overloads in embind all need to have a unique number of arguments.
  // This is invalid since both overloads take just one argument.
  function("foo", select_overload<int(int)>(&foo));
  function("foo", select_overload<int(float)>(&foo));
}
