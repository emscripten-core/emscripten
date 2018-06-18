#include <stdio.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  int sum = EM_ASM_INT({
     return $0 + $2;
  }, 0, 1, 2);
  printf("0+2=%d\n", sum);
  return 0;
}
