#include<emscripten.h>

int main() {
  EM_ASM({
    Module['setValue'](8, 1234, 'i32');
    Module['print']('|' + Module['getValue'](8, 'i32') + '|');
  });
}

