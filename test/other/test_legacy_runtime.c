#include <stdio.h>
#include <emscripten/em_asm.h>

int main () {
  intptr_t addr = EM_ASM_INT({
     return stringToNewUTF8("hello from js");
  });
  printf("%s\n", (char*)addr);
  return 0;
}
