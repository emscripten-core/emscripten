/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <xmmintrin.h>
#include <stdio.h>

int main() {
  printf("%d\n", _mm_movemask_ps(_mm_set_ps(0.f, 0.f, 0.f, -0.f)));
}
