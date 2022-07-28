#include <emscripten/emscripten.h>

int main() {
  EM_ASM({ Promise.reject("rejected!"); });
}
