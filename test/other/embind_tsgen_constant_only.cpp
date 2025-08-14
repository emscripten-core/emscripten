#include <emscripten/bind.h>

using namespace emscripten;

int SOME_CONSTANT = 15;

EMSCRIPTEN_BINDINGS(Foo) {
  constant("SOME_CONSTANT", SOME_CONSTANT);
}
