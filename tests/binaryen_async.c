#include <stdio.h>

#include <emscripten.h>

int main() {
  printf("hello, world!\n");
  int result = EM_ASM_INT_V({
    return Module.sawAsyncCompilation | 0;
  });
  REPORT_RESULT();
  return 0;
}

