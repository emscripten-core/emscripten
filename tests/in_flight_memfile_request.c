#include<stdio.h>
#include<emscripten.h>

int main() {
  int result = EM_ASM_INT_V({
    return !!Module['memoryInitializerRequest'];
  });
  printf("memory init request: %d\n", result);
  REPORT_RESULT();
  return 0;
}

