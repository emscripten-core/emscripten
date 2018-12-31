#include <stdint.h>
#include <stdio.h>
uint64_t a, b;
int main(int argc, char *argv[]) {
  a = argc;
  b = argv[1][0];
  printf("%d,%d\n", (int)a, (int)b);
  if (a > a + b || a > a + b + 1) {
    printf("one %lld, %lld", a, b);
    return 0;
  }
  printf("zero %lld, %lld", a, b);
  return 0;
}
