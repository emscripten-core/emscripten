#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

void during_exit() {
  EM_ASM({ out("during_exit 1") });
  emscripten_sleep(100);
  EM_ASM({ out("during_exit 2") });
}

int main() {
  atexit(&during_exit);
  return 0;
}
