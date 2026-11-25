/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#pragma once
#ifndef MI_ARENA_H
#define MI_ARENA_H

#include "mimalloc/util/libc.h"
#include "mimalloc/prim/prim-mte.h"  // mi_memtag_meta_init_adjacent
#include "mimalloc/arena/arena.h"
#include "bitmap.h"

bool                    _mi_arena_reserve(mi_subproc_t* subproc, size_t req_size, bool allow_large, mi_arena_id_t* arena_id);
mi_decl_nodiscard bool  _mi_arena_commit(mi_subproc_t* subproc, mi_arena_t* arena, void* start, size_t size, bool* is_zero, size_t stat_already_committed);
mi_decl_noinline  void* _mi_arenas_try_alloc(mi_heap_t* heap, size_t slice_count, size_t alignment, bool commit, bool allow_large, mi_arena_t* req_arena, size_t tseq, int numa_node, mi_memid_t* memid);
void*                   _mi_arena_os_alloc_aligned(mi_subproc_t* subproc, size_t size, size_t alignment, size_t align_offset, bool commit, bool allow_large, mi_arena_id_t req_arena_id, mi_memid_t* memid);

// Check if an arena is suitable for a request arena
static inline bool mi_arena_is_suitable(mi_arena_t* arena, mi_arena_t* req_arena) {
  if (arena == req_arena) return true;                         // they match
  if (arena == NULL) return false;
  if (req_arena == NULL && !arena->is_exclusive) return true;  // or the arena is not exclusive, and we didn't request a specific one
  mi_arena_t* const parent = mi_arena_parent(arena);
  if (parent != NULL && parent == req_arena) return true;  // sub-arena? (note that req_arena is never a sub arena)
  return false;
}

// Get the number of arenas in a subproc
static inline size_t mi_arenas_get_count(mi_subproc_t* subproc) {
  return mi_atomic_load_relaxed(&subproc->arena_count);
}

static inline mi_arena_t* mi_arena_from_index(mi_subproc_t* subproc, size_t idx) {
  mi_assert_internal(idx < mi_arenas_get_count(subproc));
  return mi_atomic_load_ptr_acquire(mi_arena_t, &subproc->arenas[idx]);
}

static inline size_t mi_arena_info_slices(mi_arena_t* arena) {
  return arena->info_slices;
}

// slices of the aligned page meta data array at the start of each aligned area
static inline size_t mi_arena_page_meta_aligned_array_slices(void) {
  #if MI_PAGE_META_IS_ALIGNED
  return mi_divide_up(MI_PAGE_META_ALIGNED_COUNT * sizeof(mi_page_t), MI_ARENA_SLICE_SIZE);
  #else
  return 0;
  #endif
}

// slices reserved at the start of each aligned area: the page meta data array (with a guard slice in front and after in secure mode)
static inline size_t mi_arena_page_meta_aligned_slice_count(void) {
  #if MI_PAGE_META_IS_ALIGNED
  return (MI_ARENA_GUARD_SLICES + mi_arena_page_meta_aligned_array_slices() + MI_ARENA_GUARD_SLICES);
  #else
  return 0;
  #endif
}

// Start of a slice
static inline uint8_t* mi_arena_slice_start(mi_arena_t* arena, size_t slice_index) {
  mi_assert_internal(slice_index < arena->slice_count);
  return (mi_arena_start(arena) + _mi_size_of_slices(slice_index));
}

// Permute within chunks: arena slice counts need not be powers of two.
static inline mi_page_t* mi_arena_page_meta_at_slice(mi_arena_t* arena, size_t slice_index) {
  mi_assert_internal(arena->pages_meta != NULL);
  mi_assert_internal(slice_index < arena->slice_count);
  mi_assert_internal((arena->slice_count % MI_BCHUNK_BITS) == 0);
  mi_assert_internal(mi_is_power_of_two(MI_BCHUNK_BITS) && (MI_PAGE_META_IDX_MUL % 2) == 1);  // (gcd(MI_BCHUNK_BITS, MI_PAGE_META_IDX_MUL) == 1) => one-to-one maping
  const size_t idx = mi_align_down(slice_index,MI_BCHUNK_BITS) + (slice_index * MI_PAGE_META_IDX_MUL) % MI_BCHUNK_BITS;   // unsigned multiply overflow is ok
  return &arena->pages_meta[idx];
}

// get the arena and slice span
static inline mi_arena_t* mi_arena_from_memid(mi_memid_t memid, size_t* slice_index, size_t* slice_count) {
  mi_assert_internal(memid.memkind == MI_MEM_ARENA);
  mi_arena_t* arena = mi_memid_arena(memid);
  if (slice_index!=NULL) { *slice_index = memid.mem.arena.slice_index; }
  if (slice_count!=NULL) { *slice_count = memid.mem.arena.slice_count; }
  return arena;
}

// With memory tagging, the tag class index of separate page meta data (alternating for adjacent page meta data)
static inline size_t mi_page_meta_memtag_idx(const mi_page_t* page) {
  return (mi_addr(page) / sizeof(mi_page_t));
}

// With memory tagging, (re)tag the separate arena page meta entry `page` with a random tag that differs from its current tag
// and the adjacent arena meta data (see `arena.c:mi_arena_memtag_init`); returns `page` with its new tag.
// The entries are preceded by the arena bitmaps, but the last entry is followed by untagged memory (or a guard page in secure mode).
static inline mi_page_t* mi_arena_page_meta_memtag(mi_arena_t* arena, mi_page_t* page) {
  const size_t i = (size_t)(mi_addr(page) - mi_addr(arena->pages_meta)) / sizeof(mi_page_t);
  mi_assert_internal(i < arena->slice_count);
  return (mi_page_t*)mi_memtag_meta_init_adjacent(page, sizeof(mi_page_t), mi_page_meta_memtag_idx(page), true /* has prev */, i + 1 < arena->slice_count /* has next */);
}

// initialize the bitmap for `slice_count` slices at base (and advance the base pointer)
// reserve a bitmap for `slice_count` slices at base (and advance the base pointer) but leave it
// uninitialized (zero chunks) until `mi_bitmap_ensure_init`; the memory must already be zero.
static inline mi_bitmap_t* mi_arena_bitmap_init_lazy(size_t slice_count, uint8_t** base) {
  mi_bitmap_t* bitmap = (mi_bitmap_t*)(*base);
  const size_t size = mi_bitmap_size(slice_count, NULL);
  *base = (*base) + size;
  return (mi_bitmap_t*)mi_ptr_bounded_data(bitmap, size);
}

static inline mi_bitmap_t* mi_arena_bitmap_init(size_t slice_count, uint8_t** base) {
  mi_bitmap_t* bitmap = mi_arena_bitmap_init_lazy(slice_count, base);
  mi_bitmap_init(bitmap, slice_count, true /* already zero */);
  return bitmap;
}

typedef bool (mi_forall_arena_fun_t)(mi_arena_t* arena, const void* arg, void** result);

bool mi_forall_arenas(mi_heap_t* heap, mi_arena_t* req_arena, size_t tseq, mi_forall_arena_fun_t* visit, const void* arg, void** result);
bool mi_forall_suitable_arenas(mi_heap_t* heap, mi_arena_t* req_arena, size_t tseq, bool match_numa, int numa_node,
                              bool allow_large, mi_forall_arena_fun_t* visit, const void* arg, void** result);


#endif
