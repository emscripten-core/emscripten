#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  int fp = atoi(argv[1]);
  printf("fp: %d\n", fp);
  void (*f)(int) = reinterpret_cast<void (*)(int)>(fp);
  f(7);
  EM_ASM({
    removeFunction($0)
  }, f);
  printf("ok\n");
  return 0;
}
