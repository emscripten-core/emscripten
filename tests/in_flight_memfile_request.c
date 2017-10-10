#include <stdio.h>
#include <emscripten.h>

int main() {
  int result = EM_ASM_INT({
    return !!Module['memoryInitializerRequest'];
  });
  printf("memory init request: %d\n", result);
  REPORT_RESULT(result);
  return 0;
}

