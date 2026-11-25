/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_ALLOC_ALLOC_H
#define MI_ALLOC_ALLOC_H

#include "../types.h"
#include "../util/libc.h"
#include "../util/options.h"

mi_decl_restrict void* _mi_page_malloc_zero(mi_theap_t* theap, mi_page_t* page, size_t size, bool zero) mi_attr_noexcept;                  // called from `_mi_theap_malloc_aligned`
bool                   _mi_page_can_reset_tail(const mi_page_t* page);
void                   _mi_page_reset_tail(const mi_page_t* page, void* p, size_t newsize, size_t usable);
mi_decl_restrict void* _mi_theap_malloc_zero(mi_theap_t* theap, size_t size, bool zero, size_t huge_alignment, mi_page_t** ppage) mi_attr_noexcept;     // called from `_mi_theap_malloc_aligned`
void*                  _mi_theap_realloc_zero(mi_theap_t* theap, void* p, size_t newsize, bool zero) mi_attr_noexcept;
void                   _mi_error_count_size_overflow(size_t count, size_t size);   // sets `errno` to `ENOMEM`

// Safe multiply `count*size` into `total`; return `true` on overflow (and set `errno` to `ENOMEM` as required by POSIX).
static inline bool mi_count_size_overflow(size_t count, size_t size, size_t* total) {
  if (count==1) {  // quick check for the case where count is one (common for C++ allocators)
    *total = size;
    return false;
  }
  else if mi_likely(!mi_mul_overflow(count, size, total)) {
    return false;
  }
  else {
    *total = SIZE_MAX;
    _mi_error_count_size_overflow(count, size);
    return true;
  }
}

// Can a reallocation to `newsize` keep a block of `size` usable bytes in `page` in place?
// Only if it fits with at most 12.5% waste, or if the new size is huge and at least half and we can
// reset the unused tail (see issue #1488).
#ifndef MI_REALLOC_SHRINK_FACTOR
#define MI_REALLOC_SHRINK_FACTOR 8     // size - (size/MI_REALLOC_SHRINK_FACTOR) is the maximum allowed waste when shrinking
#endif
static inline bool mi_realloc_is_in_place(const mi_page_t* page, size_t size, size_t newsize) {
  if mi_likely(newsize > size) return false;                                  // grow
  if mi_likely(newsize >= size - (size/MI_REALLOC_SHRINK_FACTOR)) return true; // not too much waste
  return (newsize > MI_LARGE_MAX_OBJ_SIZE && newsize >= size/2 && _mi_page_can_reset_tail(page));  // huge singleton pages
}

// Shrink a block in place; for huge blocks we reset the unused tail.
static inline void mi_realloc_shrink_in_place(const mi_page_t* page, void* p, size_t newsize, size_t size) {
  if mi_unlikely(size > MI_LARGE_MAX_OBJ_SIZE && newsize < size) { _mi_page_reset_tail(page, p, newsize, size); }
}

#endif
