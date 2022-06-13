/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>

int main() {
  // Do some printing, and make sure to use doubles and i64s too, so we test
  // their logging.
  double d = printf("some numbers: %d %f\n", 42, 2.18281828);
  uint64_t j = printf("some  more: %lld %lf\n", 1337ll, 3.14159);
  printf("counts: %lld %lf\n", j, d);
  puts("success");
  return 0;
}
