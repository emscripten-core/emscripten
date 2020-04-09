/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <stdio.h>
#include <stdint.h>

int64_t test_return64(int64_t input) {
  int64_t x = ((int64_t)1234 << 32) + 5678;
  printf("input = 0x%llx\n", input);
  return x;
}

EMSCRIPTEN_KEEPALIVE
void* get_func_ptr() {
  return &test_return64;
}
