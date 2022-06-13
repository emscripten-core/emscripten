#include <stdio.h>
#include <emscripten.h>

void *get_pc(void) { return __builtin_return_address(0); }

int magic_test_function(void) {
  int result = EM_ASM_INT({
    function report(x) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', encodeURI('http://localhost:8888?stdout=' + x));
      xhr.send();
    }
    report('magic_test_function: input=' + $0);
    var converted = wasmOffsetConverter.getName($0);
    report('magic_test_function: converted=' + converted);
    return converted == 'magic_test_function';
  }, get_pc());
  return result;
}

int main(void) {
  return magic_test_function();
}
