#include <stdio.h>

// headers test, see issue #1013
#include <cfloat>
#include <cmath>

int main(int argc, char **argv) {
  float x = 1.234, y = 3.5, q = 0.00000001;
  y *= 3;
  int z = x < y;
  printf("*%d,%d,%.1f,%d,%.4f,%.2f*\n", z, int(y), y, (int)x, x, q);

  printf("%.2f, %.2f, %.2f, %.2f\n", fmin(0.5, 3.3), fmin(NAN, 3.3),
         fmax(0.5, 3.3), fmax(NAN, 3.3));

  printf("small: %.10f\n", argc * 0.000001);

  double d = 1.12345678901234567890123e21;
  printf("double: %f\n", d);

  return 0;
}
