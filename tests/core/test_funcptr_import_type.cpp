#include <stdio.h>

extern "C" {
float floaty(float f32);
}

int main()
{
  // call it once normally. the asm.js ffi will force it to be an f64
  printf("|%f|\n", floaty(12.34));
  // call it using a function pointer, which will need to be f32
  auto* fp = floaty;
  volatile auto vfp = fp;
  printf("|%f|\n", vfp(12.34));
  return 0;
}

