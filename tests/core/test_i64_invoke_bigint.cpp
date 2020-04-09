
#include <emscripten.h>
#include <stdio.h>

long long big = 0x12345678aabbccddL;

__attribute__((noinline))
long long foobar(long long x, int y) {
  x += EM_ASM_INT({
    return 0; // prevents llvm from seeing the final value
  });
  if (x == 1337) {
    throw 1; // looks like we might throw
  }
  return x + y; // use the int parameter too, to show they are all handled
}
  
int main() {
  long long x;
  try {
    puts("try");
    x = foobar(big, 1);
  } catch(int) {
    puts("caught");
  }
  printf("ok: 0x%llx.\n", x);
}

