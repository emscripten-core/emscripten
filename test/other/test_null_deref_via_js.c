#include <emscripten.h>

int main() {
  EM_ASM({
    HEAP32[0] = 0;
  });
}
