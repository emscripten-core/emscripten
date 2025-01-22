#include <emscripten/bind.h>
#include <emscripten.h>

using namespace emscripten;

void sleep() {
  emscripten_sleep(0);
}

EMSCRIPTEN_BINDINGS(Test) {
  function("sleep", &sleep, async());
}
