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

// Attempts to geometrically or linearly increase the heap so that it
// grows by at least requested_growth_bytes new bytes. The heap size may
// be overallocated, see src/settings.js variables MEMORY_GROWTH_GEOMETRIC_STEP,
// MEMORY_GROWTH_GEOMETRIC_CAP and MEMORY_GROWTH_LINEAR_STEP. This function
// cannot be used to shrink the size of the heap.
int emscripten_resize_heap(size_t requested_size) EM_IMPORT(emscripten_resize_heap);

// Returns the current size of the WebAssembly heap.
size_t emscripten_get_heap_size(void);

// Returns the max size of the WebAssembly heap.
size_t emscripten_get_heap_max(void);

#ifdef __cplusplus
}
#endif
