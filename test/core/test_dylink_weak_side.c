#include <stdio.h>

extern int foo();

__attribute__((weak)) int foo() {
  return 99;
}

void side() {
  printf("side foo() -> %d\n", foo());
}
