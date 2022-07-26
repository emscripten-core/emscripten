#include <stdlib.h>

int main() {
  char *x = malloc(10);
  free(x);
  return x[5];
}
