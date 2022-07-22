#include <emscripten.h>

int main() {
  // Cause a JS exception
  EM_ASM({foo = missing;});
  return 0;
}
