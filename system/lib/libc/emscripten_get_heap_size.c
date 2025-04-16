/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/heap.h>
#include <stddef.h>

size_t emscripten_get_heap_size() {
  return __builtin_wasm_memory_size(0) << 16;
}
