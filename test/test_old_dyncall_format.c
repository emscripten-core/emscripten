#include <stdio.h>

void foo(int param1, int param2) {
  if (param1 == 42 && param2 == 100) printf("Test passed!\n");
}

extern void callFunc(void (*foo)(int, int), int param1, int param2);

int main() {
  callFunc(foo, 42, 100);
}
