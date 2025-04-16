/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <stdio.h>

// Prints the float/double/long double versions of the given function
// e.g.: TEST(fmax) prints fmaxf : fmax : fmaxl
#define TEST(func)                                                             \
  printf("__builtin_" #func " %f : %f : %Lf\n",                                \
         __builtin_##func##f(f1, f2),                                          \
         __builtin_##func(d1, d2),                                             \
         __builtin_##func##l(l1, l2));

#define TESTI(func)                                                            \
  printf("__builtin_" #func " %f : %f : %Lf\n",                                \
         __builtin_##func##f(f1, i1),                                          \
         __builtin_##func(d1, i1),                                             \
         __builtin_##func##l(l1, i1));

int test_builtins() {
  int i1 = 88;
  float f1 = 0.1234f;
  float f2 = 0.5678f;
  double d1 = 1.0101;
  double d2 = 0.10101;
  long double l1 = 12.0123L;
  long double l2 = 3.3201L;
  TEST(fmax)
  TEST(fmin)
  TEST(fmod)
  TEST(scalbn)
  TEST(ldexp)

  TEST(pow)
  TESTI(powi)
  return 0;
}

void test_exp_log(double x) {
  double a = exp2(x);
  double b = log10(x);
  float c = log10f(x);
  double d = acos(x);
  printf("%f : %f : %f : %d\n", a, b, c, isnan(d));
}

void test_long_double(long double x) {
  printf("sqrt(x) = %Lf\n", sqrtl(x));
  printf("ceil(x) = %Lf\n", ceill(x));
  printf("floor(x) = %Lf\n", floorl(x));
  printf("atan(x) = %Lf\n", atanl(x));
  printf("atan2(x,1) = %Lf\n", atan2l(x, 1));
  printf("asin(x) = %Lf\n", asinl(x));
  printf("acos(x) = %Lf\n", acosl(x));
  printf("expl(x) = %Lf\n", expl(x));
  printf("ldexpl(x) = %Lf\n", ldexpl(x, 7));
  printf("scalbnl(x) = %Lf\n", scalbnl(x, 2));
}

int main() {
  puts("***start***");
  test_builtins();
  test_exp_log(1234.5678);
  test_long_double(16);
  puts("***end***");
  return 0;
}
