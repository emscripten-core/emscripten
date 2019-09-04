/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
*/

#include <stdint.h>

extern intptr_t emscripten_get_sbrk_ptr();
extern int emscripten_resize_heap(size_t requested_size);

void *sbrk(intptr_t increment) {
  intptr_t old_brk = emscripten_get_sbrk_ptr();
  // TODO: overflow checks
  intptr_t updated_brk = old_brk + increment;
  uintptr_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
  if (updated_brk >= old_size) {
    // Try to grow memory.
    intptr_t diff = updated_brk - old_size;
XXX emscripten_resize_heap
    uintptr_t result = __builtin_wasm_memory_grow(0, (diff + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE);
    if (result == SIZE_MAX) {
      errno = ENOMEM;
      return (void*)-1;
    }
  }
  return (void*)old_brk;
}
