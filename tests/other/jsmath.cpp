#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
  #define TEST_1_TRIPLE(name) \
    printf("float       %s => %f\n", #name, name##f(2.1828f)); \
    printf("double      %s => %f\n", #name, name(2.1828)); \
    printf("long double %s => %Lf\n", #name, name##l(2.1828L));

  #define TEST_2_TRIPLE(name) \
    printf("float       %s => %f\n", #name, name##f(2.1828f, 3.14159f)); \
    printf("double      %s => %f\n", #name, name(2.1828, 3.14159)); \
    printf("long double %s => %Lf\n", #name, name##l(2.1828L, 3.14159L));

  TEST_1_TRIPLE(cos)
  TEST_1_TRIPLE(sin)
  TEST_1_TRIPLE(tan)
  TEST_1_TRIPLE(acos)
  TEST_1_TRIPLE(asin)
  TEST_1_TRIPLE(atan)
  TEST_2_TRIPLE(atan2)
  TEST_1_TRIPLE(exp)
  TEST_1_TRIPLE(log)
  TEST_1_TRIPLE(sqrt)
  TEST_1_TRIPLE(fabs)
  TEST_1_TRIPLE(ceil)
  TEST_1_TRIPLE(floor)
  TEST_2_TRIPLE(pow)
  TEST_1_TRIPLE(round)
  TEST_1_TRIPLE(rint)
  TEST_1_TRIPLE(nearbyint)
}
