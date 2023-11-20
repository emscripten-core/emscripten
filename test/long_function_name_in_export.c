#include <emscripten.h>

int EMSCRIPTEN_KEEPALIVE thisIsAFunctionExportedFromAsmJsOrWasmWithVeryLongFunctionNameThatWouldBeGreatToOnlyHaveThisLongNameReferredAtMostOnceInOutput(void) {
  return 1;
}

int main() {
}
