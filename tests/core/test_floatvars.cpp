/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

// headers test, see issue #1013
#include <cfloat>
#include <cmath>

#define TEST(name, x, y)     printf("    " #name "(%.2f, %.2f) => %.2f, %.2f\n", x, y, name(x, y), name(y, x));
#define TEST_ABS(name, x, y) printf("abs " #name "(%.2f, %.2f) => %.2f, %.2f\n", x, y, abs(name(x, y)), abs(name(y, x)));

// fmin etc. are not specced to be sensitive to negative zero, and LLVM does
// depend on that for optimizations, so check only the absolute value there
#define TESTS(name) \
  TEST(name, 0.33, 0.5) \
  TEST(name, NAN, 0.5) \
  TEST_ABS(name, 0.0, -0.0)

// when calling via a function pointer, the LLVM optimizer isn't involved, and
// the libc implementation does handle negative zero as you'd expect
#define PTR_TESTS(name, ptr) \
  puts("ptr " #name); \
  ptr = &name; \
  TEST(ptr, 0.33, 0.5) \
  TEST(ptr, NAN, 0.5) \
  TEST(ptr, 0.0, -0.0)

// make it look like these are externally modifiable so the optimizer doesn't
// trivially remove the function pointer
double (*funcd)(double, double) __attribute__((used));
float (*funcf)(float, float) __attribute__((used));

int main(int argc, char **argv) {
  float x = 1.234, y = 3.5, q = 0.00000001;
  y *= 3;
  int z = x < y;
  printf("*%d,%d,%.1f,%d,%.4f,%.2f*\n", z, int(y), y, (int)x, x, q);

  TESTS(fmin);
  TESTS(fmax);
  TESTS(fminf);
  TESTS(fmaxf);

  PTR_TESTS(fmin, funcd);
  PTR_TESTS(fmax, funcd);
  PTR_TESTS(fminf, funcf);
  PTR_TESTS(fmaxf, funcf);

  printf("small: %.10f\n", argc * 0.000001);

  double d = 1.12345678901234567890123e21;
  printf("double: %f\n", d);

  return 0;
}
