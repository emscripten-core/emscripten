/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

typedef int int32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef float float32x4 __attribute__((__vector_size__(16), __may_alias__));

int main() {
  float32x4 v = __builtin_convertvector((int32x4) { 255, 255, 255, 255}, float32x4);
  printf("%f\n", v[0]);
}
