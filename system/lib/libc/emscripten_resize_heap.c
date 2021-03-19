/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/heap.h>

int __emscripten_resize_heap(size_t requested_size, size_t current_size) EM_IMPORT(__emscripten_resize_heap);

// This is just a small wrapper the JavaScript function `__emscripten_resize_heap`.
// We pass in the current size to avoid reverse dependency on
// 'emscripten_get_heap_size`
int emscripten_resize_heap(size_t requested_size) {
  return __emscripten_resize_heap(emscripten_get_heap_size(), requested_size);
}
