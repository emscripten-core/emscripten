/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>

int main() {
  for (int times = 0; times < 16; ++times) {
    for (int alignment = sizeof(void*); alignment <= 64; alignment *= 2) {
      assert((uintptr_t)aligned_alloc(alignment, 64) % alignment == 0);
    }
  }
  void *ptr;
  ptr = aligned_alloc(3, 64);
  assert(ptr == NULL);
  assert(posix_memalign(&ptr, 3, 64) == EINVAL);
  assert(ptr == NULL);
  return 0;
}
