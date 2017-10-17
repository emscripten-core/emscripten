#include <emscripten.h>

int main() {
  volatile int i = 1;
  volatile int j = 0;
  EM_ASM({
    Module.print('|' + $0 + '|')
  }, i / j);
}

