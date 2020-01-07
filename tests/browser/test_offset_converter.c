#include <stdio.h>
#include <emscripten.h>

void *get_pc(void) { return __builtin_return_address(0); }

void magic_test_function(void) {
  int result = EM_ASM_INT({
    return wasmOffsetConverter.getName($0) == 'magic_test_function';
  }, get_pc());
#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}

int main(void) { magic_test_function(); }
