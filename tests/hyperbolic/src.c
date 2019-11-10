/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>

// Canonicalize nan output: wasm and asm encode negative nans differently
void printCanonicalizedNan(char* funcName, double value) {
  if (!isnan(value)) {
    printf("%s: %g\n", funcName, value);
  } else {
    printf("%s: nan\n", funcName);
  }
}

int main() {
  double i;
  for (i = -10; i < 10; i += 0.125) {
    printf("i: %g\n", i);
    printf("sinh: %g\n", sinh(i));
    printf("cosh: %g\n", cosh(i));
    printf("tanh: %g\n", tanh(i));
    printCanonicalizedNan("asinh", asinh(i));
    printCanonicalizedNan("acosh", acosh(i));
    printCanonicalizedNan("atanh", atanh(i));
    printf("\n");
  }
  return 0;
}
