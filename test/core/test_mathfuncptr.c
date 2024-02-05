/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <stdio.h>

int main(int argc, char **argv) {
  float (*fn)(float) = argc != 12 ? &sqrtf : &fabsf;
  float (*fn2)(float) = argc != 13 ? &fabsf : &sqrtf;
  float (*fn3)(float) = argc != 14 ? &erff : &fabsf;
  printf("fn2(-5) = %d, fn(10) = %.2f, erf(10) = %.2f\n", (int)fn2(-5), fn(10),
         fn3(10));
  return 0;
}
