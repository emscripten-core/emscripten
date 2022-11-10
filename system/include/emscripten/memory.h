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

// Attempts to geometrically or linearly increase the heap so that it
// grows to the new size of at least `requested_size` bytes. The heap size may
// be overallocated, see src/settings.js variables MEMORY_GROWTH_GEOMETRIC_STEP,
// MEMORY_GROWTH_GEOMETRIC_CAP and MEMORY_GROWTH_LINEAR_STEP. This function
// cannot be used to shrink the size of the heap.
int emscripten_memory_resize(size_t requested_size) EM_IMPORT(emscripten_memory_resize);

// Returns the current size of the WebAssembly memory.
size_t emscripten_memory_get_size(void);

// Returns the max size of the WebAssembly memory.
size_t emscripten_memory_get_max(void);

#ifdef __cplusplus
}
#endif
