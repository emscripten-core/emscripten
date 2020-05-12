#include <emscripten.h>
#include <stdio.h>
#include <string.h>

volatile int ten = 10;

int main(void) {
  char x[10];
  memset(x, 0, sizeof x);
  int ten = EM_ASM_INT({ return 10 });
  EM_ASM({
    var ptr = $0 + $1;
    console.log("load from", ptr);
    return HEAP8[ptr];
  }, x, ten);
  return x[ten];
}
