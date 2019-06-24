#include <stdlib.h>

int f(char *x) {
  int z = *x;
  free(x);
  return z;
}

int main() {
  char *x = malloc(10);
  static char y[10];
  x[0] = 0;
  strcpy(x, "hello");
  return f(x) + y[9];
}
