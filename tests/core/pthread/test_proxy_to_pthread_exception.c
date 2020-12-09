#include <emscripten.h>

int main() {
  EM_ASM(missingFunc());
  return 0;
}
