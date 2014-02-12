#include <stdio.h>
#include <math.h>
#include <emscripten.h>

int main() {
  volatile int x = 10;
  float y = 123456789.123456789;
  while (x-- > 0) {
    y = (sqrtf(y) + y)/2;
  }
  double d = y;
  double diff = fabs(d - 121376.4609375000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000);
  int ok = fabs(diff) < 0.000001;
  printf("%.20f : %d\n", diff, ok);

  int result;
  if (ok) result = 1;
  else result = diff+2; // add two to this >= number to avoid conflicts with 1
  REPORT_RESULT();
}

