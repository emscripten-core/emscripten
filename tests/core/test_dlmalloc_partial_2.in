#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
void *malloc(size_t size) { return (void *)123; }
int main() {
  void *x = malloc(10);
  EM_ASM_({ Module.print("got 0x" + $0.toString(16)) }, x);
  free(0);
  EM_ASM({ Module.print("freed a fake") });
  return 1;
}
