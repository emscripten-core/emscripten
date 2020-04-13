#include <stdio.h>
#include <emscripten.h>

void *get_pc(void) { return __builtin_return_address(0); }

void magic_test_function(void) {
  EM_ASM({
    var name = wasmOffsetConverter.getName($0);
    assert(name == 'magic_test_function', 'expected magic_test_function, got: ' + name);
  }, get_pc());
  puts("ok");
}

int main(void) { magic_test_function(); }
