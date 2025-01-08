#include <emscripten/bind.h>

using namespace emscripten;

void sleep() {}

EMSCRIPTEN_BINDINGS(Test) {
  function("sleep", &sleep, async());
}
