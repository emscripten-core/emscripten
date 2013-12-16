#include <stdio.h>

int main(int argc, char **argv) {
  unsigned long d1 = 0x847c9b5d;
  unsigned long q = 0x549530e1;
  if (argc > 1000) {
    q += argc;
    d1 -= argc;
  }  // confuse optimizer
  printf("%lu\n", d1 * q);
  return 0;
}
