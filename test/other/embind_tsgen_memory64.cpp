#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

long long_fn(unsigned long) { return 0; }

EMSCRIPTEN_BINDINGS(Test) {
  function("longFn", &long_fn);
}
