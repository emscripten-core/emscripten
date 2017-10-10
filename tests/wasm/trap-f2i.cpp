#include <emscripten.h>

int main() {
  volatile double d = 17179870521;
  EM_ASM({
    Module.print('|' + $0 + '|')
  }, int(d));
}

