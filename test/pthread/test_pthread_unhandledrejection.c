#include <emscripten/emscripten.h>

int main() {
  EM_ASM({ Promise.reject(new Error("rejected!")); });
  emscripten_exit_with_live_runtime();
}
