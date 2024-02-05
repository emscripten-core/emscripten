#include <stdio.h>

void side();

__attribute__((weak)) int foo() {
  return 42;
}

int main(int argc, char const *argv[]) {
  printf("main foo() -> %d\n", foo());
  side();
  return 0;
}
