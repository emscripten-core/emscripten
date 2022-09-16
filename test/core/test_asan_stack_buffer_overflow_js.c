#include <emscripten.h>
#include <string.h>

int main(void) {
  char x[10];
  memset(x, 0, sizeof x);
  return EM_ASM_INT({
    var ptr = $0 + $1;
    return HEAP8[ptr];
  }, x, 10);
}
