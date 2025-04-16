#include <string.h>

int f(int *b) {
  int a[64];
  memset(b, 0, 64 * sizeof(int));
  return f(a);
}

int main() {
  int a[64];
  f(a);
}
