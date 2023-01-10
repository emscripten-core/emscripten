#include <future>
#include <emscripten/bind.h>

using emscripten::function;

int value_from_same_thread() {
  return 1;
}

int value_from_other_thread() {
  return std::async(std::launch::async, []{
    return 2;
  }).get();
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("value_from_same_thread", &value_from_same_thread);
  function("value_from_other_thread", &value_from_other_thread);
}
