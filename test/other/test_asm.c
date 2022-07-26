#include <stdio.h>

extern int foo();

int main() {
  printf("foo -> %d\n", foo());
  return 0;
}
