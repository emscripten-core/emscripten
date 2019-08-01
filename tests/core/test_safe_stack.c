#include <string.h>

int f(int *b) {
  int a[64];
  memset(b, 0, 2048 * sizeof(int));
  return f(a);
}

int main() {
  int a[2048];
  f(a);
}
