#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// emmalloc: A lightweight web-friendly memory allocator suitable for very small applications.
// Enable the usage of emmalloc by passing the linker flag -s MALLOC=emmalloc to the application.

// A debug function that dumps the whole structure of malloc internal memory blocks to console.
// *extremely slow*, use for debugging allocation test cases.
void emmalloc_dump_memory_regions(void);

// Allocates size bytes with the given pow-2 alignment.
void *memalign(size_t alignment, size_t size);
void *emmalloc_memalign(size_t alignment, size_t size);
void *emscripten_builtin_memalign(size_t alignment, size_t size);
void *aligned_alloc(size_t alignment, size_t size);

// Allocates size bytes with default alignment (8 bytes)
void *malloc(size_t size);
void *emmalloc_malloc(size_t size);
void *emscripten_builtin_malloc(size_t size);

// Returns the number of bytes that are actually allocated to the given pointer ptr.
// E.g. due to alignment or size requirements, the actual size of the allocation can be
// larger than what was requested.
size_t malloc_usable_size(void *ptr);
size_t emmalloc_usable_size(void *ptr);

// Frees a memory pointer allocated with any of the memory allocation functions declared
// in this file, e.g.
// (emmalloc_)memalign, (emmalloc_)malloc, (emmalloc_)calloc, aligned_alloc,
// (emmalloc_)realloc, emmalloc_realloc_try, emmalloc_realloc_uninitialized, (emmalloc_)aligned_realloc
void free(void *ptr);
void emmalloc_free(void *ptr);
void emscripten_builtin_free(void *ptr);

// Performs a reallocation of the given memory pointer to a new size. If the memory region
// pointed by ptr cannot be resized in place, a new memory region will be allocated, old
// memory copied over, and the old memory area freed. The pointer ptr must have been
// allocated with one of the emmalloc memory allocation functions (malloc, memalign, ...).
// If called with size == 0, the pointer ptr is freed, and a null pointer is returned. If
// called with null ptr, a new pointer is allocated.
// If there is not enough memory, the old memory block is not freed and null pointer is
// returned.
void *realloc(void *ptr, size_t size);
void *emmalloc_realloc(void *ptr, size_t size);

// emmalloc_realloc_try() is like realloc(), but only attempts to try to resize the existing
// memory area. If resizing the existing memory area fails, then realloc_try() will return 0
// (the original memory block is not freed or modified). If resizing succeeds, previous
// memory contents will be valid up to min(old length, new length) bytes.
// If a null pointer is passed, no allocation is attempted but the function will return 0.
// If zero size is passed, the function will behave like free().
void *emmalloc_realloc_try(void *ptr, size_t size);

// emmalloc_realloc_uninitialized() is like realloc(), but old memory contents
// will be undefined after reallocation. (old memory is not preserved in any case)
void *emmalloc_realloc_uninitialized(void *ptr, size_t size);

// Like realloc(), but allows specifying the alignment to allocate to. This function cannot
// be used to change the alignment of an existing allocation, but the original pointer should
// be aligned to the given alignment already.
void *aligned_realloc(void *ptr, size_t alignment, size_t size);
void *emmalloc_aligned_realloc(void *ptr, size_t alignment, size_t size);

// emmalloc_aligned_realloc_uninitialized() is like aligned_realloc(), but old memory contents
// will be undefined after reallocation. (old memory is not preserved in any case)
void *emmalloc_aligned_realloc_uninitialized(void *ptr, size_t alignment, size_t size);

// posix_memalign allocates memory with a given alignment, like memalign, but with a slightly
// different usage signature.
int posix_memalign(void **memptr, size_t alignment, size_t size);
int emmalloc_posix_memalign(void **memptr, size_t alignment, size_t size);

// calloc allocates memory that is initialized to zero.
void *calloc(size_t num, size_t size);
void *emmalloc_calloc(size_t num, size_t size);

// mallinfo() returns information about current emmalloc allocation state. This function
// is very slow, only good for debugging. Avoid calling it for "routine" diagnostics.
struct mallinfo mallinfo();
struct mallinfo emmalloc_mallinfo();

// malloc_trim() returns unused dynamic memory back to the WebAssembly heap. Returns 1 if it
// actually freed any memory, and 0 if not. Note: this function does not release memory back to
// the system, but it only marks memory held by emmalloc back to unused state for other users
// of sbrk() to claim.
int malloc_trim(size_t pad);
int emmalloc_trim(size_t pad);

// Validates the consistency of the malloc heap. Returns non-zero and prints an error to console
// if memory map is corrupt. Returns 0 (and does not print anything) if memory is intact.
int emmalloc_validate_memory_regions(void);

// Computes the size of the dynamic memory region governed by emmalloc. This represents the
// amount of memory that emmalloc has sbrk()ed in for itself to manage. Use this function
// for memory statistics tracking purposes. Calling this function is quite fast, practically
// O(1) time.
size_t emmalloc_dynamic_heap_size(void);

// Computes the amount of memory currently reserved under emmalloc's governance  that is free
// for the application to allocate. Use this function for memory statistics tracking purposes.
// Note that calling this function is very slow, as it walks through each free memory block in
// linear time.
size_t emmalloc_free_dynamic_memory(void);

// Estimates the amount of untapped memory that emmalloc could expand its dynamic memory area
// via sbrk()ing. Theoretically the maximum amount of memory that can still be malloc()ed can
// be calculated via emmalloc_free_dynamic_memory() + emmalloc_unclaimed_heap_memory().
// Calling this function is very fast constant time lookup.
size_t emmalloc_unclaimed_heap_memory(void);

// Computes a detailed fragmentation map of available free memory. Pass in a pointer to a
// 32 element long array. This function populates into each array index i the number of free
// memory regions that have a size 2^i <= size < 2^(i+1), and returns the total number of
// free memory regions (the sum of the array entries). This function runs very slowly, as it
// iterates through all free memory blocks.
size_t emmalloc_compute_free_dynamic_memory_fragmentation_map(size_t freeMemorySizeMap[32]);

#ifdef __cplusplus
}
#endif
