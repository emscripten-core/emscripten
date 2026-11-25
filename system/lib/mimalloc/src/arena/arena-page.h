/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#pragma once
#ifndef MI_ARENA_PAGE_H
#define MI_ARENA_PAGE_H

// Shared by `arena-page.c`, `arena-page-alloc.c`, and `arena-heap.c`

#include "arena.h"

// Guard pages in front of and right after the blocks of a page (see `arena-page-alloc.c:mi_theap_page_guards`)
typedef struct mi_page_guards_s {
  size_t front;   // size of the guard page in front of the blocks (or 0)
  size_t end;     // size of the guard page right after the blocks (or 0)
} mi_page_guards_t;


/* -----------------------------------------------------------
  Arena pages and page meta data
----------------------------------------------------------- */

// Restrict a pointer to page meta data to the page meta data only
static inline mi_page_t* mi_page_meta_bounded(void* p) {
  return (mi_page_t*)mi_ptr_bounded_data(p, sizeof(mi_page_t));
}

static inline mi_arena_pages_t* mi_heap_arena_pages(mi_heap_t* heap, mi_arena_t* arena) {
  mi_assert_internal(arena!=NULL);
  mi_assert_internal(heap!=NULL);
  mi_assert(arena->arena_idx < MI_MAX_ARENAS);
  return mi_atomic_load_ptr_acquire(mi_arena_pages_t, &heap->arena_pages[arena->arena_idx]);
}


// defined in `arena-page.c`
mi_arena_t*       _mi_page_arena_pages(mi_page_t* page, size_t* slice_index, size_t* slice_count, mi_arena_pages_t** parena_pages);
mi_page_t*        mi_arena_page_at_slice(mi_arena_t* arena, size_t slice_index);
mi_page_t*        _mi_arenas_page_try_find_abandoned(mi_theap_t* theap, size_t slice_count, size_t block_size, mi_page_guards_t guards);
mi_arena_pages_t* _mi_heap_ensure_arena_pages(mi_heap_t* heap, mi_arena_t* arena);
bool              _mi_abandoned_page_unown(mi_page_t* page, mi_theap_t* current_theapx);

// defined in `arena-page-alloc.c`
void              _mi_arenas_page_free_prim(mi_page_t* page);  // free a page without modifying page_bin stats

#endif
