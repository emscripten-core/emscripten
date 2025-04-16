/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <stdio.h>

void printCanonicalizedNan(char* name, float value) {
  if (!isnan(value)) {
    printf("%s: %f\n", name, value);
  } else {
    printf("%s: nan\n", name);
  }
}

int main(int argc, const char* argv[]) {
  float f1 = 1.0f;
  float f2 = 0.0f;
  float f_zero = 0.0f;

  float f3 = 0.0f / f2;
  float f4 = f2 / 0.0f;
  float f5 = f2 / f2;
  float f6 = f2 / f_zero;

  printCanonicalizedNan("f3", f3);
  printCanonicalizedNan("f4", f4);
  printCanonicalizedNan("f5", f5);
  printCanonicalizedNan("f6", f6);

  return 0;
}
