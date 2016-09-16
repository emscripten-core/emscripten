#include <stdio.h>

int main() {
  printf("%o %o %o %o %o\n", 10, 256, 1023, 1001001, 12345678);
  printf("%#o %#o %#o %#o %#o\n", 10, 256, 1023, 1001001, 12345678);
  return 0;
}
