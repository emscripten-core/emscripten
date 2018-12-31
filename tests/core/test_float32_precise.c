#include <stdio.h>

int main(int argc, char **argv) {
  float x = 1.23456789123456789;
  float y = 5.20456089123406709;
  while (argc > 10 || argc % 19 == 15) {
    // confuse optimizer
    x /= y;
    y = 2 * y - 1;
    argc--;
  }
  x = x - y;
  y = 3 * y - x / 2;
  x = x * y;
  y += 0.000000000123123123123;
  x -= y / 7.654;
  printf("\n%.20f, %.20f\n", x, y);
  return 0;
}
