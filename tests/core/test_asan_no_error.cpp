#include <stdlib.h>

int f(int *x) {
  int z = *x;
  delete [] x;
  return z;
}

int main() {
  int *x = new int[10];
  static char y[10];
  return ((f(x) + y[9]) % 16) + 1;
}
