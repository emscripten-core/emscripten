#include<emscripten.h>

int main() {
#ifdef DIRECT
  EM_ASM({
    setValue(8, 1234, 'i32');
    Module['print']('|' + getValue(8, 'i32') + '|');
  });
#else
  EM_ASM({
    Module['setValue'](8, 1234, 'i32');
    Module['print']('|' + Module['getValue'](8, 'i32') + '|');
  });
#endif
}

