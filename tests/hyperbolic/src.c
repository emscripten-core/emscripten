#include <stdio.h>
#include <math.h>

int main() {
  double i;
  for (i = -10; i < 10; i += 0.125) {
    printf("i: %g\n", i);
    printf("sinh: %g\n", sinh(i));
    printf("cosh: %g\n", cosh(i));
    printf("tanh: %g\n", tanh(i));
    printf("asinh: %g\n", asinh(i));
    printf("acosh: %g\n", acosh(i));
    printf("atanh: %g\n", atanh(i));
    printf("\n");
  }
  return 0;
}
