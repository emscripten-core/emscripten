#include <emscripten.h>

int main() {
  // Copy a field around to mark to JavaScript code that we have successfully completed the main() of this program.
  EM_ASM(Module.weHaveExecutedSomeCCode = Module.inputData);
}
