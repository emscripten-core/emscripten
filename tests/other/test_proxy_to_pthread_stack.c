#include <stdint.h>
#include <stdlib.h>
#include <emscripten.h>
int main(void) {
  int32_t data[64*1024];
  EM_ASM({ console.log($0, "success"); }, data);
  exit(0);
}
