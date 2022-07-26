/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>

// test copysign of 0
int __attribute__((noinline))
copysign_bug (double x)
{
  if (x != 0.0 && (x * 0.5 == x)) {
    printf("1\n");
    return 1;
  }
  printf("f: %f\n", x);
  if (__builtin_copysign(1.0, x) < 0.0) {
    printf("2\n");
    return 2;
  } else {
    printf("3\n");
    return 3;
  }
}

int main() {
#define TEST(x, y) printf("%.2f, %.2f ==> %.2f\n", x, y, copysign(x, y));
  TEST(5.0f, 5.0f);
  TEST(5.0f, -5.0f);
  TEST(-5.0f, 5.0f);
  TEST(-5.0f, -5.0f);
  TEST(5.0f, 4.0f);
  TEST(5.0f, -4.0f);
  TEST(-5.0f, 4.0f);
  TEST(-5.0f, -4.0f);
  TEST(0.0f, 5.0f);
  TEST(0.0f, -5.0f);
  TEST(-0.0f, 5.0f);
  TEST(-0.0f, -5.0f);
  TEST(5.0f, 0.0f);
  TEST(5.0f, -0.0f);
  TEST(-5.0f, 0.0f);
  TEST(-5.0f, -0.0f);
  TEST(0.0f, 0.0f);
  TEST(0.0f, -0.0f);
  TEST(-0.0f, 0.0f);
  TEST(-0.0f, -0.0f);

  double x = -0.0;
  if (copysign_bug (x) != 2)
    __builtin_abort ();

  return 0;
}
