#include <stdlib.h>
#include <stdio.h>

void f1() {
  printf("f1\n");
  __builtin_trap();
}

void f2() {
  printf("f2\n");
}

int main() {
  atexit(f1);
  at_quick_exit(f2);
  quick_exit(0);
  __builtin_trap();
}
