#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

uint64_t bigint_fn(int64_t) { return 0; }

EMSCRIPTEN_BINDINGS(Test) {
  function("bigintFn", &bigint_fn);
}
