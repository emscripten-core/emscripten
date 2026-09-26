/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/emscripten.h>
#include <stdio.h>
#include <math.h>

EMSCRIPTEN_KEEPALIVE float add_f32(float a, float b) {
  return a + b;
}

EMSCRIPTEN_KEEPALIVE int64_t add_i64(int64_t a, int64_t b) {
  return a + b;
}

extern float import_f32(float x);

extern int64_t import_i64(int64_t x);

int main () {
  float a = 1.2;
  float b = import_f32((float)3.4);
  float c;
  c = add_f32(a, b);

  int64_t d = 1;
  int64_t e = import_i64((int64_t)2);
  int64_t f;
  f = add_i64(d, e);
}
