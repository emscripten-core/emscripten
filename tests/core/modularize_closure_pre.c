#include <emscripten.h>

int main() {
  EM_ASM({
    Module['print'](Module['on_module']);
  });
}

