#include <stdio.h>

typedef int (*F)();

int side_module_trampoline_b(F f);

int side_module_trampoline_a(F f) {
  printf("side_module_trampoline_a\n");
  return side_module_trampoline_b(f);
}
