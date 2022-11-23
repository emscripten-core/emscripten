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
// variable (the end of the dynamic memory region). This address stays the same
// throughout program lifetime.
uintptr_t *emscripten_get_sbrk_ptr(void);

// Attempts to geometrically or linearly increase the heap so that it
// grows to the new size of at least `requested_size` bytes. The heap size may
// be overallocated, see src/settings.js variables MEMORY_GROWTH_GEOMETRIC_STEP,
// MEMORY_GROWTH_GEOMETRIC_CAP and MEMORY_GROWTH_LINEAR_STEP. This function
// cannot be used to shrink the size of the heap.
int emscripten_resize_heap(size_t requested_size) EM_IMPORT(emscripten_resize_heap);

// Returns the base address of the global data section. The global data section
// contains the constant global and static function local data, either zero- or
// non-zero initialized. This address stays constant throughout program lifetime.
uintptr_t emscripten_get_global_base(void);

// Returns the end address of the global data section. N.b. the memory location
// pointed to by this end value is not part of the global data section itself, i.e.
// the global data section consists of the half-open range [global_base,
// global_end[ in the usual begin-end pointer span style. This address stays
// constant throughout program lifetime.
uintptr_t emscripten_get_global_end(void);

// Returns the starting address of the dynamic memory heap. This value corresponds
// to the the sbrk value at program startup, when no dynamic memory allocations
// have yet been made. This address stays constant throughout program lifetime.
uintptr_t emscripten_get_dyn_heap_base(void);

// Returns the end address of the dynamic memory heap (same as the sbrk() point)
// This value can change during program execution, as the WebAssembly heap is
// grown by calling sbrk().
#define emscripten_get_dyn_heap_end() (*emscripten_get_sbrk_ptr())

// Returns the current size of the WebAssembly heap (the WebAssembly.Memory object).
// This value can change during program execution, as the WebAssembly heap is
// grown by calling emscripten_resize_heap().
size_t emscripten_get_heap_size(void);

// Returns the max size of the WebAssembly heap (the WebAssembly.Memory object).
// This is the largest size that the memory is allowed to grow up to, set at
// compile time. This value is a constant throughout program lifetime.
size_t emscripten_get_heap_max(void);

// Direct access to the system allocator.  Use these to access that underlying
// allocator when intercepting/wrapping the allocator API.  Works with with both
// dlmalloc and emmalloc.
void *emscripten_builtin_memalign(size_t alignment, size_t size);
void *emscripten_builtin_malloc(size_t size);
void emscripten_builtin_free(void *ptr);

#ifdef __cplusplus
}
#endif
