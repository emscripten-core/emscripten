#include <stdio.h>

int IsNaN(double x) {
  int rc; /* The value return */
  volatile double y = x;
  volatile double z = y;
  rc = (y != z);
  return rc;
}

int main() {
  double tests[] = {1.0, 3.333, 1.0 / 0.0,  0.0 / 0.0, -1.0 / 0.0,
                    -0,  0,     -123123123, 12.0E200};
  for (int i = 0; i < sizeof(tests) / sizeof(double); i++)
    printf("%d - %f - %d\n", i, tests[i], IsNaN(tests[i]));
}
