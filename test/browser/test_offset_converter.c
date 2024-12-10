#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

void *get_pc(void) {
  return __builtin_return_address(0);
}

void magic_test_function(void) {
  int result = EM_ASM_INT({
    function report(x) {
      out(x);
      fetch('http://localhost:8888?stdout=' + encodeURIComponent(x));
    }
    report('magic_test_function: input=' + $0);
    var converted = wasmOffsetConverter.getName($0);
    report('magic_test_function: converted=' + converted);
    return converted == 'magic_test_function';
  }, get_pc());
  assert(result);
}

int main(void) {
  magic_test_function();
  return 0;
}
