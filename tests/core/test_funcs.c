#include <stdio.h>
int funcy(int x) { return x * 9; }
int main() {
  printf("*%d,%d*\n", funcy(8), funcy(10));
  return 0;
}
