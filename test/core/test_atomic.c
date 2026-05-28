/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main() {
  int x = 10;
  int y = __sync_add_and_fetch(&x, 5);
  printf("*%d,%d*\n", x, y);
  x = 10;
  y = __sync_fetch_and_add(&x, 5);
  printf("*%d,%d*\n", x, y);
  x = 10;
  y = __sync_lock_test_and_set(&x, 6);
  printf("*%d,%d*\n", x, y);
  x = 10;
  y = __sync_bool_compare_and_swap(&x, 9, 7);
  printf("*%d,%d*\n", x, y);
  y = __sync_bool_compare_and_swap(&x, 10, 7);
  printf("*%d,%d*\n", x, y);

  // Test 128-bit CAS
  __int128 val128 = 0;
  __int128 expected = 0;
  __int128 desired = 42;
  __int128 result = __sync_val_compare_and_swap(&val128, expected, desired);
  printf("__sync_val_compare_and_swap: %lld %lld\n", (uint64_t)result, (uint64_t)val128);

  return 0;
}
