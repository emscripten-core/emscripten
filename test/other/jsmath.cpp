#include <emscripten.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// Get values through a call to JS, so the optimizer can't hardcode results.
EM_JS(double, get_number, (), {
  return 2.1828;
});

EM_JS(double, get_another_number, (), {
  return 3.1415;
});

int main() {
  #define TEST_1_TRIPLE(name) \
    printf("float       %s => %f\n", #name, name##f((float)get_number())); \
    printf("double      %s => %f\n", #name, name(get_number()));

  #define TEST_2_TRIPLE(name) \
    printf("float       %s => %f\n", #name, name##f((float)get_number(), get_another_number())); \
    printf("double      %s => %f\n", #name, name(get_number(), get_another_number()));

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
