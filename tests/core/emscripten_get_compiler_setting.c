#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

int main() {
  printf("QS: %d\n", emscripten_get_compiler_setting("QUANTUM_SIZE"));
  assert((unsigned)emscripten_get_compiler_setting("OPT_LEVEL") <= 3);
  assert((unsigned)emscripten_get_compiler_setting("DEBUG_LEVEL") <= 4);
  printf("EV: %s\n", (char*)emscripten_get_compiler_setting("EMSCRIPTEN_VERSION"));
}

