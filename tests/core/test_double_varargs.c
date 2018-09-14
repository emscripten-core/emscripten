/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>

double func_int_double_1(int unused1, ...)
{
  int i;
  double d;
  va_list vl;
  va_start(vl, unused1);
  i = va_arg(vl, int);
  d = va_arg(vl, double);
  va_end(vl);
  return i+d;
}

double func_int_double_2(int unused1, int unused2, ...)
{
  int i;
  double d;
  va_list vl;
  va_start(vl, unused2);
  i = va_arg(vl, int);
  d = va_arg(vl, double);
  va_end(vl);
  return i+d;
}

int main() {
  double ret = func_int_double_1(0, 5, 10.0);
  printf("%f\n", ret); // Expects to print 15
  ret = func_int_double_2(0, 0, 5, 10.0);
  printf("%f\n", ret); // Expects to print 15
}

