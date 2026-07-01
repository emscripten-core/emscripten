#include <emscripten.h>
#include <emscripten/bind.h>

int primary_function() {
  return 42;
}

int deferred_function() {
  return 82;
}

EMSCRIPTEN_BINDINGS(module_splitting) {
  emscripten::function("primary_function", &primary_function);
  emscripten::function("deferred_function", &deferred_function, emscripten::async());
}
