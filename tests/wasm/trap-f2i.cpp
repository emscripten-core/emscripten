#include <emscripten.h>

int main() {
  volatile double d = 17179870521;
  EM_ASM_({
    Module.print('|' + $0 + '|')
  }, int(d));
}

