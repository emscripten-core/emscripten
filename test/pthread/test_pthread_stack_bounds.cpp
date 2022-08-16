#include <cassert>
#include <thread>
#include <emscripten.h>
#include <emscripten/stack.h>

void thread(void) {
  bool passed;
  size_t stack_base = emscripten_stack_get_base();
  size_t stack_max = emscripten_stack_get_end();
  size_t current = (size_t) &passed;
  assert(stack_base > current && current > stack_max);
  emscripten_force_exit(0);
}

int main(void) {
  std::thread t(thread);
  t.detach();
  emscripten_exit_with_live_runtime();
  __builtin_trap();
}
