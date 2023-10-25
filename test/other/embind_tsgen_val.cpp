#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

int global_fn(int, int) { return 0; }

EMSCRIPTEN_BINDINGS(Test) {
  function("global_fn", &global_fn);
}

int main() {
  val::global("window").call<val>("setTimeout");
  // Main should not be run during TypeScript generation.
  abort();
  return 0;
}
