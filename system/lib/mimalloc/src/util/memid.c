/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
Memory id's describe how a memory area was allocated (OS, arena, static, etc.)
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/util/libc.h"       // _mi_memzero_var
#include "mimalloc/prim/prim-mte.h"   // _mi_os_has_memtag
#include "mimalloc/arena/arena.h"     // _mi_size_of_slices
#include "mimalloc/util/memid.h"

mi_memid_t _mi_memid_create(mi_memkind_t memkind) {
  mi_memid_t memid;
  _mi_memzero_var(memid);
  memid.memkind = memkind;
  return memid;
}

mi_memid_t _mi_memid_none(void) {
  return _mi_memid_create(MI_MEM_NONE);
}

mi_memid_t _mi_memid_create_os(void* base, size_t size, size_t commit_size, bool is_zero, bool is_large) {
  mi_memid_t memid = _mi_memid_create(MI_MEM_OS);
  memid.mem.os.base = base;
  memid.mem.os.psize = mi_memid_os_pages(size);
  memid.mem.os.pcommitted = mi_memid_os_pages(commit_size);
  memid.initially_committed = (commit_size > 0);
  memid.initially_zero = is_zero;
  memid.is_pinned = is_large;
  memid.is_memtag = (!is_large && _mi_os_has_memtag());
  return memid;
}

mi_memid_t _mi_memid_create_static(void* p, size_t size) {
  mi_memid_t memid = _mi_memid_create(MI_MEM_STATIC);
  memid.mem.malloc.base = p;
  memid.mem.malloc.size = size;
  memid.initially_committed = true;
  memid.is_pinned = true;
  return memid;
}

mi_memid_t _mi_memid_create_malloc(void* p, size_t size, bool iszero) {
  mi_memid_t memid = _mi_memid_create(MI_MEM_MALLOC);
  memid.mem.malloc.base = p;
  memid.mem.malloc.size = size;
  memid.initially_committed = true;
  memid.initially_zero = iszero;
  memid.is_pinned = true;
  return memid;
}

size_t _mi_memid_size(mi_memid_t memid) {
  if (mi_memid_is_os(memid)) {
    return mi_memid_os_size(memid);
  }
  else if (memid.memkind == MI_MEM_ARENA) {
    return _mi_size_of_slices(memid.mem.arena.slice_count);
  }
  else if (memid.memkind == MI_MEM_MALLOC) {
    return memid.mem.malloc.size;
  }
  else {
    mi_assert_internal(mi_memid_needs_no_free(memid));
    return 0;
  }
}
