#include <stdio.h>

int main() {
  char c = '1';
  short s = 2;
  int i = 3;
  long long l = 4;
  float f = 5.5;
  double d = 6.6;

  printf("%c,%hd,%d,%lld,%.1f,%.1f\n", c, s, i, l, f, d);
  printf("%#x,%#x\n", 1, 0);

  return 0;
}
