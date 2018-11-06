// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <math.h>

// We have to use a proxy function 'acos_test' here because the updated libc++ library provides a set of overloads to acos,
// this has the result that we can't take the function pointer to acos anymore due to failed overload resolution.
// This proxy function has no overloads so it's allowed to take the function pointer directly.
double acos_test(double x) {
  return acos(x);
}

typedef double (*ddd)(double x, double unused);
typedef int    (*iii)(int x,    int unused);

int main() {
  volatile ddd d = (ddd)acos_test;
  volatile iii i = (iii)acos_test;
  printf("|%.3f,%d|\n", d(0.3, 0.6), i(0, 0));
  return 0;
}
