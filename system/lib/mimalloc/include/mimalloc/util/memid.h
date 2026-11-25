/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_UTIL_MEMID_H
#define MI_UTIL_MEMID_H

#include "../types.h"


// defined in util/memid.c
mi_memid_t    _mi_memid_create(mi_memkind_t memkind);
mi_memid_t    _mi_memid_none(void);
mi_memid_t    _mi_memid_create_os(void* base, size_t size, size_t commit_size, bool is_zero, bool is_large);
mi_memid_t    _mi_memid_create_static(void* p, size_t size);
mi_memid_t    _mi_memid_create_malloc(void* p, size_t size, bool iszero);
size_t        _mi_memid_size(mi_memid_t memid);


static inline bool mi_memid_is_os(mi_memid_t memid) {
  return mi_memkind_is_os(memid.memkind);
}

static inline bool mi_memid_os_size_fits(size_t size) {
  #if (MI_SIZE_BITS <= 32 + MI_MEMID_OS_PAGE_SHIFT)
  MI_UNUSED(size);
  return true;   // always fits (and avoid a -Wtype-limits warning)
  #else
  return ((size >> MI_MEMID_OS_PAGE_SHIFT) <= UINT32_MAX);
  #endif
}

static inline uint32_t mi_memid_os_pages(size_t size) {
  mi_assert_internal(size % MI_MEMID_OS_PAGE_SIZE == 0 && mi_memid_os_size_fits(size));
  return (uint32_t)(size >> MI_MEMID_OS_PAGE_SHIFT);
}

static inline size_t mi_memid_os_size(mi_memid_t memid) {
  return ((size_t)memid.mem.os.psize << MI_MEMID_OS_PAGE_SHIFT);
}

static inline size_t mi_memid_os_committed(mi_memid_t memid) {
  return ((size_t)memid.mem.os.pcommitted << MI_MEMID_OS_PAGE_SHIFT);
}

static inline void mi_memid_os_set_committed(mi_memid_t* memid, size_t commit_size) {
  mi_assert_internal(mi_memid_is_os(*memid) && commit_size <= mi_memid_os_size(*memid));
  memid->mem.os.pcommitted = mi_memid_os_pages(commit_size);
}

static inline bool mi_memid_needs_no_free(mi_memid_t memid) {
  return mi_memkind_needs_no_free(memid.memkind);
}

#endif
