#include <stdlib.h>
#include <stdio.h>

int f(int *x) {
  int z = *x;
  delete [] x;
  return z;
}

int main() {
  int *x = new int[10];
  static char y[10];

  printf("%d %d\n", f(x), y[9]);
  printf("done\n");
  return 0;
}
