/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

static const double tol = 1e-16;

void test_value(double value) {
  int exponent;
  double x = frexp(value, &exponent);
  double expected = x * pow(2.0, exponent);

  printf("%f=%f*2^%d\n", value, x, exponent);

  assert(fabs(expected - value) < tol);
  assert(x == 0 || (fabs(x) >= 5e-1 && fabs(x) < 1));  // x has a magnitude in
                                                       // the interval [1/2, 1)
}

int main() {
  test_value(0);
  test_value(100.1);
  test_value(-100.1);
  test_value(.5);
  test_value(-.5);
  test_value(1 - 1e-16);
  test_value(-(1 - 1e-16));

  return 0;
}
