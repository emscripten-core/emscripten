#include <stdio.h>
int test_wrapper(void);

int side_module_trampoline() {
  printf("side_module_trampoline\n");
  return test_wrapper();
}
