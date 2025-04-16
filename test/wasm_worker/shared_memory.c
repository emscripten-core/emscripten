#include <emscripten.h>
#include <stdio.h>

int main() {
  int is_shared = EM_ASM_INT(return wasmMemory.buffer instanceof SharedArrayBuffer);
  printf("%d\n", is_shared);
  return 0;
}
