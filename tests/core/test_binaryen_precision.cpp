#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <limits>

int main() {
  printf("begin:\n");
  // test double-to-int conversions along the limits
  double x = std::numeric_limits<int>::min();
  printf("min-0.5: %d\n", int(x - 0.5));
  printf("min    : %d\n", int(x));
  printf("min+0.5: %d\n", int(x + 0.5));
  printf("min+1  : %d\n", int(x + 1));
  printf("min+2  : %d\n", int(x + 2));
  double y = std::numeric_limits<int>::max();
  printf("max-2  : %d\n", int(y - 2));
  printf("max-1  : %d\n", int(y - 1));
  printf("max-0.5: %d\n", int(y - 0.5));
  printf("max    : %d\n", int(y));
  printf("max+0.5: %d\n", int(y + 0.5));
  // the undefined behaviors. native builds can emit -min_int or 0, accept both
  #define TEST(d) { \
    int temp = d; \
    if (temp != 0 && temp != std::numeric_limits<int>::min()) { \
      printf("bad: int(%s) => %d\n", #d, temp); \
      abort(); \
    } \
  }
  TEST(x - 2);
  TEST(x - 1);
  TEST(y + 1);
  TEST(y + 2);
  TEST(-std::numeric_limits<double>::infinity());
  TEST(+std::numeric_limits<double>::infinity());
  TEST(-std::numeric_limits<double>::quiet_NaN());
  printf("ok.\n");
}

