#include <stdio.h>

#include <emscripten.h>

int main() {
  printf("hello, world!\n");
  int result = EM_ASM_INT({
    return Module.sawAsyncCompilation | 0;
  });
  REPORT_RESULT(result);
  return 0;
}

