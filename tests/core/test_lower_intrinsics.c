/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <emscripten.h>

int main() {
  volatile float f = 1.0f;
  volatile double d = 2.0;

  #define TEST1(name) { \
    volatile float tf = name##f(f); \
    volatile double td = name(d); \
  }
  TEST1(cos);
  TEST1(exp);
  TEST1(log);
  TEST1(sin);
  TEST1(sqrt);

  #define TEST2(name) { \
    volatile float tf = name##f(f, f); \
    volatile double td = name(d, d); \
  }
  TEST2(pow);

  EM_ASM({
    out('ok.');
  });
}

