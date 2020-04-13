#include <thread>
#include <emscripten.h>

void thread(void) {
  bool passed;
  size_t stack_base = EM_ASM_INT({ return STACK_BASE; });
  size_t stack_max = EM_ASM_INT({ return STACK_MAX; });
  size_t current = (size_t) &passed;
#if __asmjs__
  passed = stack_base < current && current < stack_max;
#else
  passed = stack_base > current && current > stack_max;
#endif
#ifdef REPORT_RESULT
  REPORT_RESULT(passed ? 1 : 0);
#endif
}

int main(void) {
  std::thread t(thread);
  t.detach();
}
