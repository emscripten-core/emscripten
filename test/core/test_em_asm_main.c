#include <stdio.h>
#include <emscripten.h>

int test_side();

int main() {
  printf("in main\n");
  EM_ASM(out("em_asm in main"));
  return test_side();
}
