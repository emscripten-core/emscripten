/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
  char* endptr;
  printf("\n");
  printf("%g\n", strtod("0", &endptr));
  printf("%g\n", strtod("0.", &endptr));
  printf("%g\n", strtod("0.0", &endptr));
  printf("%g\n", strtod("-0.0", &endptr));
  printf("%g\n", strtod("1", &endptr));
  printf("%g\n", strtod("1.", &endptr));
  printf("%g\n", strtod("1.0", &endptr));
  printf("%g\n", strtod("z1.0", &endptr));
  printf("%g\n", strtod("0.5", &endptr));
  printf("%g\n", strtod(".5", &endptr));
  printf("%g\n", strtod(".a5", &endptr));
  printf("%g\n", strtod("123", &endptr));
  printf("%g\n", strtod("123.456", &endptr));
  printf("%g\n", strtod("-123.456", &endptr));
  printf("%g\n", strtod("1234567891234567890", &endptr));
  printf("%g\n", strtod("1234567891234567890e+50", &endptr));
  printf("%g\n", strtod("84e+220", &endptr));
  printf("%g\n", strtod("123e-50", &endptr));
  printf("%g\n", strtod("123e-250", &endptr));
  printf("%g\n", strtod("123e-450", &endptr));
  printf("%g\n", strtod("0x6", &endptr));
  printf("%g\n", strtod("-0x0p+0", &endptr));
  char str[] = "  12.34e56end";
  printf("%g\n", strtod(str, &endptr));
  printf("%zu\n", endptr - str);
  printf("%g\n", strtod("84e+420", &endptr));
  printf("%.12f\n", strtod("1.2345678900000000e+08", NULL));
  return 0;
}
