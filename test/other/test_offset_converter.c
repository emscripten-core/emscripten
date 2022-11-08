#include <stdio.h>
#include <emscripten.h>

void *get_pc(void) {
  return __builtin_return_address(0);
}

void magic_test_function(void) {
  void* pc = get_pc(); // This is line (9) and on which we fetch the PC (at column 14).
  EM_ASM({
    var pc = $0;

    var name = wasmOffsetConverter.getName(pc);
    console.log('wasmOffsetConverter ' + ptrToString(pc) + ' -> ' + name);
    assert(name == 'magic_test_function', 'expected magic_test_function, got: ' + name);

#ifdef USE_SOURCE_MAP
    // In addtion to wasmOffsetConverter (which only uses the name section)
    // we can also use the source map to get more accurate line info.
    assert(typeof wasmSourceMap !== 'undefined');
    source = wasmSourceMap.lookup(pc);
    assert(source);
    console.log(source);
    assert(source.file.includes('test_offset_converter.c'));
    assert(source.line == 9);
    assert(source.column == 14);
#endif
  }, pc);
  puts("ok");
}

EM_JS_DEPS(test, "$ptrToString");

int main(void) {
  magic_test_function();
  return 0;
}
