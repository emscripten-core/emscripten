/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <xmmintrin.h>

char buffer[21];

int main() {
  _mm_storeu_ps((float*)&buffer[1], (__m128){0.1, 2.3, 4.5, 6.7});
  __m128 y = _mm_loadu_ps((float*)&buffer[5]);

  float __attribute__((__aligned__(16))) ar[4];
  *(__m128 *)&ar = y;
  for (int i = 0; i < 4; i++) {
    printf("%f\n", ar[i]);
  }

  return 0;
}
