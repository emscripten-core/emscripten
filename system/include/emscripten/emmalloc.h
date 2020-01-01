#pragma once

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

// Frees a memory pointer allocated with any of
// emmalloc_memalign, emmalloc_malloc, 
void free(void *ptr);
void emmalloc_free(void *ptr);
void emscripten_builtin_free(void *ptr);

// Performs a reallocation of the given memory pointer to a new size. If the memory region
// pointed by ptr cannot be resized in place, a new memory region will be allocated, old
// memory copied over, and the old memory area freed. The pointer ptr must have been
// allocated with one of the emmalloc memory allocation functions (malloc, memalign, ...).
// If called with size == 0, the pointer ptr is freed, and a null pointer is returned. If
// called with null ptr, a new pointer is allocated.
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

// malloc_trim() frees up unused memory back to the system. Note: currently not implemented,
// but only provided for cross-compilation compatibility. Always returns 0.
int malloc_trim(size_t pad);
int emmalloc_malloc_trim(size_t pad);

#ifdef __cplusplus
}
#endif
