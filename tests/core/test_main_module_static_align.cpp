#include <emscripten.h>

__attribute__((aligned(16))) volatile char aligned;

int main() {
  emscripten_log(EM_LOG_WARN, "16-byte aligned char: %d.", int(&aligned));
  EM_ASM({
    // whether the char was properly aligned affects tempDoublePtr, which is after the static aligns
    Module['print']('tempDoublePtr: ' + tempDoublePtr + '.');
    Module['print']('tempDoublePtr alignment: ' + (tempDoublePtr % 16) + '.');
  });
}

