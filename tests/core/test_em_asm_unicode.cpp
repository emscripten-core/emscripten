#include <emscripten.h>

int main() {
  EM_ASM( out("hello world…") );
}
