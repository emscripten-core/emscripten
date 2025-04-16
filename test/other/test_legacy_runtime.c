#include <stdio.h>
#include <emscripten/em_asm.h>

int main () {
  intptr_t addr = EM_ASM_INT({
     return allocate(intArrayFromString("hello from js"), ALLOC_NORMAL);
  });
  printf("%s\n", (char*)addr);
  return 0;
}
