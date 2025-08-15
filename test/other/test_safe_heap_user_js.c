#include <emscripten.h>

int main() {
  EM_ASM({
    HEAP8[0] = 42;
  });
}
