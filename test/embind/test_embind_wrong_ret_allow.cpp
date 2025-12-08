#include <emscripten/bind.h>

using namespace emscripten;

class C {};

void passThrough(C* ptr) {}

EMSCRIPTEN_BINDINGS(raw_pointers) {
  class_<C>("C");
  function("passThrough", &passThrough, allow_raw_pointer<ret_val>());
}
