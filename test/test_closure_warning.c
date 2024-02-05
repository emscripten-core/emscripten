#include <emscripten.h>

int main() {
  EM_ASM(foo = 2; var foo; out(foo));
}
