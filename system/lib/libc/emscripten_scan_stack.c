#include <emscripten.h>
#include <emscripten/stack.h>

void emscripten_scan_stack(em_scan_func func) {
  uintptr_t base = emscripten_stack_get_base();
  uintptr_t end = emscripten_stack_get_current();
  func((void*)end, (void*)base);
}
