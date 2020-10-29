#include <thread>
#include <emscripten.h>
#include <emscripten/stack.h>

void thread(void) {
  bool passed;
  size_t stack_base = emscripten_stack_get_base();
  size_t stack_max = emscripten_stack_get_end();
  size_t current = (size_t) &passed;
  passed = stack_base > current && current > stack_max;
#ifdef REPORT_RESULT
  REPORT_RESULT(passed ? 1 : 0);
#endif
}

int main(void) {
  std::thread t(thread);
  t.detach();
}
