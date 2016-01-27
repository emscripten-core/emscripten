#include <stdio.h>
#include <math.h>

int main() {
  printf("%.1f ", round(1.4));
  printf("%.1f ", round(1.6));
  printf("%.1f ", round(-1.4));
  printf("%.1f ", round(-1.6));

  printf("%.1f ", round(1.5));
  printf("%.1f ", round(2.5));
  printf("%.1f ", round(-1.5));
  printf("%.1f ", round(-2.5));

  printf("%ld ", lrint(1.4));
  printf("%ld ", lrint(1.6));
  printf("%ld ", lrint(-1.4));
  printf("%ld ", lrint(-1.6));

  printf("%ld ", lrint(1.5));
  printf("%ld ", lrint(2.5));
  printf("%ld ", lrint(-1.5));
  printf("%ld ", lrint(-2.5));

  printf("\n");

  double param, fractpart, intpart;

  param = 3.14159265;
  fractpart = modf (param , &intpart);
  printf ("%f = %f + %f \n", param, intpart, fractpart);

  param = -3.14159265;
  fractpart = modf (param , &intpart);
  printf ("%f = %f + %f \n", param, intpart, fractpart);

  printf("%.1f ", roundf(1.4));
  printf("%.1f ", roundf(1.6));
  printf("%.1f ", roundf(-1.4));
  printf("%.1f ", roundf(-1.6));

  printf("%.1f ", roundf(1.5));
  printf("%.1f ", roundf(2.5));
  printf("%.1f ", roundf(-1.5));
  printf("%.1f ", roundf(-2.5));

  return 0;
}
