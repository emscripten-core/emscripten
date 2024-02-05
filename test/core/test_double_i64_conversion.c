/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

__attribute((noinline)) int eq(double d, int64_t i) {
  int64_t i2 = (int64_t)d;
  if (i != i2) {
    printf("%.20g converted to int64 returns %lld, not %lld as expected!\n", d,
           i2, i);
  }
  return i == i2;
}

int main() {
  assert(eq(0.0, 0));
  assert(eq(-0.0, 0));
  assert(eq(0.1, 0));
  assert(eq(-0.1, 0));
  assert(eq(0.6, 0));
  assert(eq(-0.6, 0));
  assert(eq(1.0, 1));
  assert(eq(-1.0, -1));
  assert(eq(1.1, 1));
  assert(eq(-1.1, -1));
  assert(eq(1.6, 1));
  assert(eq(-1.6, -1));
  assert(eq(4294967295.0, 4294967295LL));
  assert(eq(4294967295.5, 4294967295LL));
  assert(eq(4294967296.0, 4294967296LL));
  assert(eq(4294967296.5, 4294967296LL));
  assert(eq(14294967295.0, 14294967295LL));
  assert(eq(14294967295.5, 14294967295LL));
  assert(eq(14294967296.0, 14294967296LL));
  assert(eq(14294967296.5, 14294967296LL));
  assert(eq(-4294967295.0, -4294967295LL));
  assert(eq(-4294967295.5, -4294967295LL));
  assert(eq(-4294967296.0, -4294967296LL));
  assert(eq(-4294967296.5, -4294967296LL));
  assert(eq(-14294967295.0, -14294967295LL));
  assert(eq(-14294967295.5, -14294967295LL));
  assert(eq(-14294967296.0, -14294967296LL));
  assert(eq(-14294967296.5, -14294967296LL));

  assert(eq(4294967295.3, 4294967295LL));
  assert(eq(4294967296.3, 4294967296LL));
  assert(eq(14294967295.3, 14294967295LL));
  assert(eq(14294967296.3, 14294967296LL));
  assert(eq(-4294967295.3, -4294967295LL));
  assert(eq(-4294967296.3, -4294967296LL));
  assert(eq(-14294967295.3, -14294967295LL));
  assert(eq(-14294967296.3, -14294967296LL));

  assert(eq(4294967295.8, 4294967295LL));
  assert(eq(4294967296.8, 4294967296LL));
  assert(eq(14294967295.8, 14294967295LL));
  assert(eq(14294967296.8, 14294967296LL));
  assert(eq(-4294967295.8, -4294967295LL));
  assert(eq(-4294967296.8, -4294967296LL));
  assert(eq(-14294967295.8, -14294967295LL));
  assert(eq(-14294967296.8, -14294967296LL));

  // The following number is the largest double such that all integers smaller
  // than this can exactly be represented in a double.
  assert(eq(9007199254740992.0, 9007199254740992LL /* == 2^53 */));
  assert(eq(-9007199254740992.0, -9007199254740992LL /* == -2^53 */));

  printf("OK!\n");
  return 0;
}
