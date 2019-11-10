/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <inttypes.h>
#include <stdio.h>

int main() {
  uint64_t x = 0, y = 0;
  for (int i = 0; i < 64; i++) {
    x += 1ULL << i;
    y += x;
    x /= 3;
    y *= 5;
    printf("unsigned %d: %llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", i, x, y, x+y, x-y, x*y, y ? x/y : 0, x ? y/x : 0, y ? x%y : 0, x ? y%x : 0);
  }
  int64_t x2 = 0, y2 = 0;
  for (int i = 0; i < 64; i++) {
    x2 += 1LL << i;
    y2 += x2;
    x2 /= 3 * (i % 7 ? -1 : 1);
    y2 *= 5 * (i % 2 ? -1 : 1);
    printf("signed %d: %lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", i, x2, y2, x2+y2, x2-y2, x2*y2, y2 ? x2/y2 : 0, x2 ? y2/x2 : 0, y2 ? x2%y2 : 0, x2 ? y2%x2 : 0);
  }
  return 0;
}
