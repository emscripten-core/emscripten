/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <emscripten/emscripten.h>
#include <emscripten/memory.h>
#include <stdint.h>
#include <stddef.h>

#define WASM_PAGE_SIZE 65536
#define EMSCRIPTEN_PAGE_SIZE WASM_PAGE_SIZE

#ifdef __cplusplus
extern "C" {
#endif

// Returns a pointer to a memory location that contains the heap DYNAMICTOP
// variable (the end of the dynamic memory region)
uintptr_t *emscripten_get_sbrk_ptr(void);

// Direct access to the system allocator.  Use these to access that underlying
// allocator when intercepting/wrapping the allocator API.  Works with with both
// dlmalloc and emmalloc.
void *emscripten_builtin_memalign(size_t alignment, size_t size);
void *emscripten_builtin_malloc(size_t size);
void emscripten_builtin_free(void *ptr);

#define emscripten_resize_heap(X) _Pragma("GCC warning \"'emscripten_resize_heap' is deprecated.  Please use 'emscripten_memory_resize'\"") emscripten_memory_resize(X)
#define emscripten_get_heap_size() _Pragma("GCC warning \"'emscripten_get_heap_size' is deprecated.  Please use 'emscripten_memory_resize'\"") emscripten_memory_get_size()
#define emscripten_get_heap_max() _Pragma("GCC warning \"'emscripten_get_heap_max' is deprecated.  Please use 'emscripten_memory_resize'\"") emscripten_memory_get_max()

#ifdef __cplusplus
}
#endif
