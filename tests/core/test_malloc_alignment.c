/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef ALIGN8
#define EXPECTED_ALIGNMENT 8
#else
#define EXPECTED_ALIGNMENT alignof(max_align_t)
#endif

int main(int argc, char **argv) {
  bool underaligned = false;

  for (int size = 0; size < 16; ++size) {
    void *p = malloc(size);
    assert(((uintptr_t)p) % EXPECTED_ALIGNMENT == 0);
    if (((uintptr_t)p) % alignof(max_align_t) != 0) {
      underaligned = true;
    }
  }

#if ALIGN8
  // Ensure that we have have at least one allocation that is under 16-byte
  // alignment when using the align8 variants of malloc.
  assert(underaligned);
#endif
  return 0;
}
