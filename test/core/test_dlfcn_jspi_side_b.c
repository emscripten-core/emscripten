#include <stdio.h>
int test_wrapper(void);

int side_module_trampoline_b() {
  printf("side_module_trampoline_b\n");
  return test_wrapper();
}
