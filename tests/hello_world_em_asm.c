#include <emscripten.h>

int main() {
  EM_ASM({ Module.print("hello, world!\n"); });
  return 0;
}

