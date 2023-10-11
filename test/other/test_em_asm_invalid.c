#include <emscripten/em_asm.h>

int main() {
  EM_ASM({
    * this is not valid js *
  });
}
