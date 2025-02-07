#include <stdio.h>
int side_module_trampoline_b(void);

int side_module_trampoline_a() {
  printf("side_module_trampoline_a\n");
  return side_module_trampoline_b();
}
