#include <stdio.h>

typedef long long int64;
#define JSDOUBLE_HI32_SIGNBIT 0x80000000

bool JSDOUBLE_IS_NEGZERO(double d) {
  union {
    struct {
      unsigned int lo, hi;
    } s;
    double d;
  } x;
  if (d != 0) return false;
  x.d = d;
  return (x.s.hi & JSDOUBLE_HI32_SIGNBIT) != 0;
}

bool JSINT64_IS_NEGZERO(int64 l) {
  union {
    int64 i;
    double d;
  } x;
  if (l != 0) return false;
  x.i = l;
  return x.d == -0;
}

int main(int argc, char* argv[]) {
  printf("*%d,%d,%d,%d*\n", JSDOUBLE_IS_NEGZERO(0), JSDOUBLE_IS_NEGZERO(-0),
         JSDOUBLE_IS_NEGZERO(-1), JSDOUBLE_IS_NEGZERO(+1));
  printf("*%d,%d,%d,%d*\n", JSINT64_IS_NEGZERO(0), JSINT64_IS_NEGZERO(-0),
         JSINT64_IS_NEGZERO(-1), JSINT64_IS_NEGZERO(+1));
  return 0;
}
