#include <stdio.h>

typedef int (*F)();

int side_module_trampoline_b(F f) {
  printf("side_module_trampoline_b\n");
  return f();
}
