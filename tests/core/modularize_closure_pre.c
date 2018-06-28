#include <emscripten.h>

int main() {
  EM_ASM({
    out(Module['on_module']);
  });
}

