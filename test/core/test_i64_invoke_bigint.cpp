#include <emscripten.h>
#include <stdio.h>
#include <stdint.h>

int64_t big = 0x12345678aabbccddL;

__attribute__((noinline))
int64_t foobar(int64_t x, int y) {
  x += EM_ASM_INT({
    return 0; // prevents llvm from seeing the final value
  });
  if (x == 1337) {
    throw 1; // looks like we might throw
  }
  return x + y; // use the int parameter too, to show they are all handled
}

int main() {
  int64_t x;
  try {
    puts("try");
    x = foobar(big, 1);
    printf("ok: 0x%llx.\n", x);
    x = foobar(1337, 1);
    printf("should not get here\n");
    __builtin_trap();
  } catch(int e) {
    printf("caught: %d\n", e);
  }
  printf("done");
  return 0;
}
