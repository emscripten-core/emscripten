/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
Allocate a page (`_mi_arenas_page_alloc`): either reclaim an abandoned page, or allocate the memory
and meta data of a fresh arena or OS page and initialize it. Also frees pages again.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/prim/prim-tls.h"
#include "mimalloc/prim/prim-mte.h"
#include "mimalloc/util/stats.h"
#include "mimalloc/util/memid.h"
#include "mimalloc/alloc/page.h"
#include "mimalloc/arena/arena-alloc.h"
#include "mimalloc/profile/sample-guarded.h"  // _mi_page_unguard_all
#include "mimalloc/arena/arena-page.h"
#include "arena-page.h"

/* -----------------------------------------------------------
  Guard pages in front of and right after the blocks of a page.
  These are used in secure mode for the pages of meta theaps,
  at the end of singleton pages (huge blocks), and at the end of all pages at secure level 5.
  The `memid.guard_front/end` flags record the guard pages of a page.
----------------------------------------------------------- */

// The guard pages a theap uses for a fresh page of `size` bytes. To limit the overhead, we only use guard pages
// if together they take at most 1/4 of the page. OS aligned pages have no front guard page.
// There are no guard pages if the theap allocates from an exclusive pinned arena (as pinned memory cannot be protected).
static mi_page_guards_t mi_theap_page_guards(const mi_theap_t* theap, size_t size, bool singleton, bool os_align) {
  const size_t gsize = _mi_os_secure_guard_page_size();
  const bool guard_end = (theap->guard_end || (singleton && MI_SECURE >= 1));  // in secure mode, huge blocks are always followed by a guard page
  mi_page_guards_t guards = { 0, 0 };
  const mi_arena_t* const req_arena = mi_theap_heap(theap)->exclusive_arena;
  if (req_arena != NULL && req_arena->memid.is_pinned) return guards;
  if (theap->guard_front && !os_align && gsize <= size/4) { guards.front = gsize; }
  if (guard_end && guards.front + gsize <= size/4) { guards.end = gsize; }
  return guards;
}

// The offset of the first block after a front guard page (of at least 4 KiB which is the maximal natural block alignment)
static size_t mi_page_guard_front_block_start(size_t guard_front, size_t block_size) {
  return mi_align_up(guard_front, mi_bounds_align(block_size));  // on CHERI, keep exact bounds
}

// Can we offset the first block with a large cache offset to spread cache sets? (see `MI_PAGE_LARGE_CACHE_OFFSET`)
static bool mi_page_block_can_use_large_cache_offset(size_t block_size) {
  if (!mi_block_size_has_large_cache_offset(block_size)) return false;
  #if MI_GUARDED
  if (block_size % _mi_os_page_size() == 0) return false;  // guarded blocks must stay OS page aligned
  #endif
  return true;
}


/* -----------------------------------------------------------
  Allocate the memory area and page meta data for a fresh page
  (`mi_arenas_page_alloc_fresh_area`)
----------------------------------------------------------- */

// Offset of the first block when the page meta data is in front of the slices.
static size_t mi_page_meta_in_front_block_start(size_t block_size, bool os_align) {
  size_t offset;
  #if MI_GUARDED
  // in a guarded build, we align pages with blocks a multiple of an OS page size, to the OS page size
  // this ensures that all blocks in such pages are OS page size aligned (which is needed for the guard pages)
  const size_t os_page_size = _mi_os_page_size();
  mi_assert_internal(MI_PAGE_ALIGN >= os_page_size);
  if (!os_align && block_size % os_page_size == 0 && block_size > os_page_size /* at least 2 or more */ ) {
    offset = mi_align_up(mi_page_info_size(), os_page_size);
  }
  else
  #endif
  if (os_align) {
    offset = MI_PAGE_ALIGN;
  }
  else if (block_size != 0 && mi_is_power_of_two(block_size) && block_size <= MI_PAGE_MAX_START_BLOCK_ALIGN2) {
    // naturally align power-of-2 blocks up to MI_PAGE_MAX_START_BLOCK_ALIGN2 size (4KiB)
    offset = mi_align_up(mi_page_info_size(), block_size);
  }
  else if (block_size != 0 && block_size <= MI_SMALL_SIZE_MAX) {
    // align small blocks to their size
    offset = mi_align_up(mi_page_info_size(), block_size);
  }
  else if (block_size != 0 && (block_size % MI_PAGE_OSPAGE_BLOCK_ALIGN2) == 0) {
    // align blocks that are a multiple of MI_PAGE_OSPAGE_BLOCK_ALIGN2 (4KiB) (or to the cache offset unit)
    const size_t alignment = (mi_page_block_can_use_large_cache_offset(block_size) ? MI_PAGE_LARGE_CACHE_OFFSET_UNIT : MI_PAGE_OSPAGE_BLOCK_ALIGN2);
    offset = mi_align_up(mi_page_info_size(), alignment);
  }
  else {
    // otherwise start after the info
    offset = mi_page_info_size();
  }
  // on CHERI, also align such that every block has exact bounds (os_align is already aligned enough)
  return mi_align_up(offset, (os_align ? MI_MAX_ALIGN_SIZE : mi_max(MI_MAX_ALIGN_SIZE, mi_bounds_align(block_size))));
}

static size_t mi_page_cache_offset_index(uint8_t* slice_start, size_t slice_count, size_t noffsets) {
  // Fibonacci hash of the page index: the top bits spread consecutive pages evenly over the offsets
  const size_t page_index = (size_t)(mi_addr(slice_start) / MI_ARENA_SLICE_SIZE) / slice_count;
  #if MI_SIZE_BITS >= 64
  const size_t hash = page_index * MI_ZU(0x9E3779B97F4A7C15);
  #else
  const size_t hash = page_index * MI_ZU(0x9E3779B9);
  #endif
  return (((hash >> (MI_SIZE_BITS - 16)) * noffsets) >> 16);
}

static size_t mi_page_small_block_cache_offset(uint8_t* slice_start, size_t slice_count, size_t block_size) {
  const size_t unit = mi_max(MI_PAGE_SMALL_CACHE_OFFSET_UNIT, mi_natural_alignment_of_size(block_size));
  if (unit > MI_PAGE_SMALL_CACHE_OFFSET_MAX) return 0;
  return mi_page_cache_offset_index(slice_start, slice_count, 1 + MI_PAGE_SMALL_CACHE_OFFSET_MAX/unit) * unit;
}

// Cache offset for the first block to spread the blocks of different pages over the cache sets (issue #1121).
// The offset fits in the unused space at the end of the page; if there is none, we skip the last block
// if that wastes at most 12.5% of the page.
static size_t mi_page_block_cache_offset(uint8_t* slice_start, size_t slice_count, size_t block_size, size_t block_start, bool os_align, size_t page_noguard_size) {
  if (MI_PAGE_LARGE_CACHE_OFFSET == 0 || os_align) return 0;
  if (!mi_block_size_has_large_cache_offset(block_size)) return mi_page_small_block_cache_offset(slice_start, slice_count, block_size);
  if (!mi_page_block_can_use_large_cache_offset(block_size)) return 0;
  const size_t unit = mi_max(MI_PAGE_LARGE_CACHE_OFFSET_UNIT, mi_bounds_align(block_size));  // on CHERI, keep exact bounds
  if (unit > MI_PAGE_LARGE_CACHE_OFFSET_MAX) return 0;
  mi_assert_internal(block_start + block_size <= page_noguard_size);
  size_t slack = (page_noguard_size - block_start) % block_size;
  if (slack < unit && block_size <= page_noguard_size/8) { slack += block_size; }      // offset 0 still fits exact
  const size_t noffsets = 1 + (slack < MI_PAGE_LARGE_CACHE_OFFSET_MAX ? slack : MI_PAGE_LARGE_CACHE_OFFSET_MAX) / unit;
  return mi_page_cache_offset_index(slice_start, slice_count, noffsets) * unit;
}

// Use the separate (zero initialized) `page_meta` if available and not preferring in front of the slices,
// and otherwise put the page meta data in front of the slices (without a front guard page).
// Also determines the offset of the first block.
static mi_page_t* mi_page_meta_select(uint8_t* slice_start, size_t slice_count, mi_page_t* page_meta, size_t block_size, bool os_align,
                                      mi_page_guards_t* guards, size_t* block_start) {
  mi_assert_internal(page_meta == NULL || page_meta->block_size == 0);
  mi_page_t* page;
  size_t offset;
  if (page_meta == NULL
      #if MI_PAGE_META_SMALL_IS_ALIGNED
      // small pages keep the page meta data in front of the slices for a fast `mi_free_small`
      // (note: it is important that the separate `page_meta->block_size == 0` for `mi_arena_page_at_slice`)
      || (!os_align && block_size <= MI_SMALL_MAX_OBJ_SIZE)
      #endif
  ) {
    // put the actual page meta info in front of the slices
    // (note: it is not initialized yet since the slices may not be committed yet)
    offset = mi_page_meta_in_front_block_start(block_size, os_align);
    page = (mi_page_t*)slice_start;
    guards->front = 0;
  }
  else {
    // use the separate page meta data
    mi_assert_internal(guards->front == 0 || guards->front >= MI_PAGE_MAX_START_BLOCK_ALIGN2);
    mi_memzero_aligned(page_meta, sizeof(*page_meta));
    offset = mi_page_guard_front_block_start(guards->front, block_size);
    page = page_meta;
  }
  offset += mi_page_block_cache_offset(slice_start, slice_count, block_size, offset, os_align, _mi_size_of_slices(slice_count) - guards->end);
  mi_assert_internal(os_align || offset < MI_ARENA_SLICE_SIZE);  // so `mi_page_slice_start` can align down the page start
  *block_start = offset;
  return mi_page_meta_bounded(page);
}

#if MI_PAGE_META_IS_ALIGNED
// Ensure the aligned page meta data for the slices at `slice_start` is committed.
static bool mi_arena_page_meta_ensure_committed(mi_arena_t* arena, uint8_t* slice_start) {
  uint8_t* const meta_slices = (uint8_t*)mi_align_down_ptr(slice_start,MI_PAGE_META_ALIGNMENT) + MI_PAGE_META_ALIGNED_OFFSET;
  mi_assert_internal(meta_slices >= mi_arena_start(arena));
  const size_t meta_slice_index  = (meta_slices - mi_arena_start(arena)) / MI_ARENA_SLICE_SIZE;
  if mi_unlikely(mi_bitmap_is_clear(arena->slices_committed, meta_slice_index)) {
    // try to commit all page meta slices now (but not the guard slices around it)
    const size_t meta_slice_count = mi_arena_page_meta_aligned_array_slices();
    // the following assertion does not hold in a concurrent setting..
    // mi_assert_internal(mi_bitmap_is_clearN(arena->slices_committed, meta_slice_index, meta_slice_count));
    const size_t commit_size = meta_slice_count * MI_ARENA_SLICE_SIZE;
    if (!_mi_arena_commit(arena->subproc, arena, meta_slices, commit_size, NULL, 0)) {
      return false;
    }
    mi_bitmap_setN(arena->slices_committed, meta_slice_index, meta_slice_count, NULL);
  }
  return true;
}
#endif

// Try to allocate slices from free space in the arena's, and return the separate page meta data (if available) in `*ppage_meta`.
static uint8_t* mi_arenas_page_alloc_from_slices(mi_theap_t* theap, size_t slice_count, bool commit, bool allow_large,
                                                 mi_memid_t* memid, mi_arena_pages_t** parena_pages, mi_page_t** ppage_meta) {
  mi_heap_t* const heap = mi_theap_heap(theap);
  mi_tld_t*  const tld  = theap->tld;
  const int numa_node = (heap->numa_node >= 0 ? heap->numa_node : tld->numa_node);

  uint8_t* const start = (uint8_t*)_mi_arenas_try_alloc(heap, slice_count, MI_ARENA_SLICE_ALIGN, commit, allow_large, heap->exclusive_arena, tld->thread_seq, numa_node, memid);
  if (start == NULL) return NULL;
  mi_arena_t* const arena = mi_memid_arena(*memid);

  // ensure the arena pages (and the aligned page meta data)
  mi_arena_pages_t* const arena_pages = _mi_heap_ensure_arena_pages(heap, arena);
  if (arena_pages==NULL
      #if MI_PAGE_META_IS_ALIGNED
      || !mi_arena_page_meta_ensure_committed(arena, start)
      #endif
     ) {
    _mi_arenas_free(heap->subproc, start, _mi_size_of_slices(slice_count), *memid); // roll back
    return NULL;
  }
  // note: the following assert should hold if we could check it atomically, but in a concurrent setting we may already allocate in slice_count
  // mi_assert_internal(mi_bitmap_is_clearN(arena_pages->pages, memid->mem.arena.slice_index, memid->mem.arena.slice_count));
  mi_assert_internal(mi_bitmap_is_clear(arena_pages->pages, memid->mem.arena.slice_index));
  // don't set yet: mi_bitmap_set(arena_pages->pages, memid->mem.arena.slice_index);
  *parena_pages = arena_pages;

  // the separate page meta data (if available)
  #if MI_PAGE_META_IS_ALIGNED
  *ppage_meta = mi_aligned_ptr_page0(start);
  #else
  mi_assert_internal(MI_PAGE_META_IS_SEPARATED!=0 || arena->pages_meta == NULL);
  *ppage_meta = (arena->pages_meta == NULL ? NULL : mi_page_meta_load(mi_arena_page_meta_at_slice(arena, memid->mem.arena.slice_index)));
  #endif
  return start;
}

#if MI_PAGE_META_OS_IS_SEPARATED
#if MI_PAGE_META_IS_ALIGNED
// The size of the aligned page meta data up to and including the entries for a page at `page_offset` in an aligned OS area.
static size_t mi_page_meta_os_aligned_size(size_t page_offset, size_t max_page_meta_count) {
  size_t count = 0;
  const size_t slice_end = mi_divide_up(page_offset + MI_ARENA_SLICE_SIZE, MI_ARENA_SLICE_SIZE) + max_page_meta_count;
  for (size_t slice_idx = page_offset / MI_ARENA_SLICE_SIZE; slice_idx < slice_end; slice_idx++) {
    count = mi_max(count, mi_page_meta_aligned_index(slice_idx) + 1);
  }
  return count * sizeof(mi_page_t);
}
#endif

// Allocate slices from the OS with the (committed and zero'd) page meta data in front of the page area (returned in `*ppage_meta`).
// The OS memory is `[guard slice | page meta data | guard slice | ... | page area]` where the guard slices are only reserved in secure mode.
// With aligned page meta data, the OS memory is `MI_PAGE_META_ALIGNMENT` aligned and the page meta data is indexed by slice (see `mi_aligned_ptr_page`).
static uint8_t* mi_arenas_page_alloc_from_os(mi_heap_t* heap, size_t slice_count, size_t max_page_meta_count, size_t block_alignment, bool os_align, bool commit, bool allow_large,
                                             mi_memid_t* memid, mi_page_t** ppage_meta) {
  MI_UNUSED(allow_large); MI_UNUSED(max_page_meta_count); MI_UNUSED(os_align);
  mi_arena_t* const req_arena = heap->exclusive_arena;
  const size_t alloc_size = _mi_size_of_slices(slice_count);
  const size_t page_alignment = (block_alignment < MI_PAGE_ALIGN ? MI_PAGE_ALIGN : block_alignment);
  #if MI_PAGE_META_IS_ALIGNED
  if (block_alignment >= MI_PAGE_META_ALIGNMENT) {
    // if we allow alignment >= MI_PAGE_META_ALIGNMENT we need to substract 1 from a pointer
    // in mi_aligned_ptr_page (and test for (intptr_t)p < 0 instead of NULL). We avoid this by limiting the max alignment.
    _mi_warning_message("requested alignment is too large (%zu KiB)\n", block_alignment / MI_KiB);
    errno = EINVAL;
    return NULL;
  }
  const size_t os_alignment = MI_PAGE_META_ALIGNMENT;
  #else
  const size_t os_alignment = page_alignment;
  #endif

  // offset of the page area (leaving room for the guard slices around the page meta data)
  size_t page_offset = mi_align_up(_mi_size_of_slices(2*MI_ARENA_GUARD_SLICES + 1), page_alignment);
  #if MI_PAGE_META_IS_ALIGNED
  size_t page_meta_size = mi_page_meta_os_aligned_size(page_offset, max_page_meta_count);
  while (_mi_size_of_slices(MI_ARENA_GUARD_SLICES) + mi_align_up(page_meta_size,_mi_os_page_size()) + _mi_os_secure_guard_page_size() > page_offset) {
    // the spread page meta entries need more room in front of the page area
    page_offset += page_alignment;
    page_meta_size = mi_page_meta_os_aligned_size(page_offset, max_page_meta_count);
  }
  #else
  const size_t page_meta_size = sizeof(mi_page_t);
  #endif
  uint8_t* const os_start = (uint8_t*)_mi_arena_os_alloc_aligned(heap->subproc, alloc_size + page_offset, os_alignment, 0 /* align offset */, false /* commit */, false /* allow large */, req_arena, memid);
  if (os_start==NULL) return NULL;
  uint8_t* const start = os_start + page_offset;

  // commit the page meta data and page area
  uint8_t* const page_meta_start = os_start + _mi_size_of_slices(MI_ARENA_GUARD_SLICES);
  mi_assert_internal(page_meta_start + mi_align_up(page_meta_size,_mi_os_page_size()) + _mi_os_secure_guard_page_size() <= start);
  bool is_zero;
  bool ok = _mi_arena_commit(heap->subproc,req_arena,page_meta_start,page_meta_size,&is_zero,page_meta_size /* don't count in stats? */);
  if (ok && commit) {
    ok = _mi_arena_commit(heap->subproc,req_arena,start,alloc_size,NULL,0);
  }
  if (!ok) { _mi_os_free(heap->subproc,os_start,alloc_size+page_offset,*memid); return NULL; }
  if (!is_zero && !memid->initially_zero) {
    mi_memzero_aligned(page_meta_start,page_meta_size);
  }
  #if MI_ARENA_GUARD_SLICES > 0
  // set guard pages around the page meta data (if not initially committed, the guards are never committed)
  if (memid->initially_committed && !memid->is_pinned) {
    _mi_os_secure_guard_pages_set_around(heap->subproc, page_meta_start, page_meta_size, *memid);
  }
  #endif
  if (commit) {
    memid->initially_committed = true;  // so we don't commit again
    mi_memid_os_set_committed(memid, alloc_size);
  }

  // and determine the page meta data
  mi_assert_internal(mi_is_aligned(start,page_alignment));
  #if MI_PAGE_META_IS_ALIGNED
  mi_assert_internal(mi_is_aligned(os_start,MI_PAGE_META_ALIGNMENT));
  mi_assert_internal(mi_align_down_ptr(start-1,MI_PAGE_META_ALIGNMENT) == os_start);
  mi_assert_internal((uint8_t*)mi_aligned_ptr_page0(start) < page_meta_start + page_meta_size);
  *ppage_meta = mi_aligned_ptr_page0(start);
  #else
  *ppage_meta = (mi_page_t*)mi_ptr_bounded_data(page_meta_start, sizeof(mi_page_t));
  #endif
  return start;
}
#else
// Allocate slices from the OS with the page meta data in front of the slices (so there is no separate page meta data).
static uint8_t* mi_arenas_page_alloc_from_os(mi_heap_t* heap, size_t slice_count, size_t max_page_meta_count, size_t block_alignment, bool os_align, bool commit, bool allow_large,
                                             mi_memid_t* memid, mi_page_t** ppage_meta) {
  MI_UNUSED(max_page_meta_count);
  const size_t page_alignment = MI_ARENA_SLICE_ALIGN;
  const size_t alloc_size = _mi_size_of_slices(slice_count);
  uint8_t* start;
  if (os_align) {
    // note: slice_count already includes the page meta data (of MI_PAGE_ALIGN size) in front of the aligned block
    start = (uint8_t*)_mi_arena_os_alloc_aligned(heap->subproc, alloc_size, block_alignment, page_alignment /* align offset */, commit, allow_large, heap->exclusive_arena, memid);
    mi_assert_internal(start==NULL || mi_is_aligned(start + page_alignment, block_alignment));
  }
  else {
    start = (uint8_t*)_mi_arena_os_alloc_aligned(heap->subproc, alloc_size, page_alignment, 0 /* align offset */, commit, allow_large, heap->exclusive_arena, memid);
  }
  if (start == NULL) return NULL;
  *ppage_meta = NULL;
  return start;
}
#endif

// Allocate the slices for a fresh page and return its separate page meta data (if available) in `*ppage_meta`.
static uint8_t* mi_arenas_page_alloc_fresh_area(mi_theap_t* theap, size_t slice_count, size_t max_page_meta_count, size_t block_alignment, bool os_align, bool commit, bool allow_large,
                                                mi_memid_t* memid, mi_arena_pages_t** parena_pages, mi_page_t** ppage_meta ) {
  mi_assert_internal(parena_pages!=NULL && ppage_meta!=NULL);
  *parena_pages = NULL;
  *ppage_meta = NULL;
  *memid = _mi_memid_none();
  mi_heap_t* const heap = mi_theap_heap(theap);

  // try to allocate from free space in arena's
  uint8_t* slice_start = NULL;
  if (!mi_option_is_enabled(mi_option_disallow_arena_alloc) &&       // allowed to allocate from arena's?
      !os_align &&                                                   // not large alignment
      slice_count <= mi_arena_max_object_size()/MI_ARENA_SLICE_SIZE) // and not too large
  {
    slice_start = mi_arenas_page_alloc_from_slices(theap, slice_count, commit, allow_large, memid, parena_pages, ppage_meta);
  }

  // otherwise fall back to the OS
  if (slice_start == NULL) {
    slice_start = mi_arenas_page_alloc_from_os(heap, slice_count, max_page_meta_count, block_alignment, os_align, commit, allow_large, memid, ppage_meta);
    if (slice_start == NULL) return NULL;
    mi_heap_stat_increase(heap,pages_os_allocated,1);
    mi_atomic_increment_relaxed(&heap->os_page_count);
  }
  mi_assert_internal(mi_is_aligned(slice_start, MI_PAGE_ALIGN));
  return slice_start;
}


/* -----------------------------------------------------------
  Allocate and initialize a fresh page (`mi_arenas_page_alloc_fresh`)
----------------------------------------------------------- */

// Commit the start of the page up to and including the first block if the slices are not yet committed.
// Sets `*commit_size` to the committed size (or 0 if the slices were already committed).
// Never commits beyond `page_end` (the end guard page or the end of the page).
static bool mi_arenas_page_commit_first_block(mi_theap_t* theap, mi_memid_t memid, uint8_t* slice_start, size_t block_start, size_t block_size, size_t page_end, size_t* commit_size) {
  *commit_size = 0;
  if (memid.initially_committed) return true;
  mi_assert_internal(!memid.guard_front);  // as the front guard page would never be recommitted (see `_mi_arenas_page_free_prim`)
  size_t csize = mi_align_up(block_start + block_size, mi_page_min_commit_size());
  if (csize > page_end) { csize = page_end; }
  bool is_zero = false;
  if mi_unlikely(!_mi_arena_commit(mi_theap_subproc(theap), mi_memid_arena(memid), slice_start, csize, &is_zero, 0)) {
    return false;
  }
  *commit_size = csize;
  return true;
}

// Prepare the page memory: zero the page meta data if it is in front of the slices, set the guard pages, etc.
// The end guard page (if any) is at `page_end`.
static void mi_arenas_page_init_memory(mi_theap_t* theap, mi_page_t* page, uint8_t* slice_start, size_t slice_count, size_t block_start, size_t page_end, mi_memid_t* memid)
{
  MI_UNUSED(slice_count); MI_UNUSED(block_start);

  // zero initialize the page meta data
  if (!memid->initially_zero && (uint8_t*)page == slice_start) {
    mi_memzero_aligned(page, sizeof(*page));
  }

  // claimed free slices: initialize the page partly
  if (!memid->initially_zero && memid->initially_committed) {
    mi_track_mem_undefined(slice_start, slice_count * MI_ARENA_SLICE_SIZE);
  }
  else if (memid->initially_committed) {
    mi_track_mem_defined(slice_start, slice_count * MI_ARENA_SLICE_SIZE);
  }
  #if MI_DEBUG > 1
  if (memid->initially_zero && memid->initially_committed) {
    if (!mi_mem_is_zero(slice_start, page_end)) {
      _mi_error_message(EFAULT, "internal error: page memory was not zero initialized.\n");
      memid->initially_zero = false;
      if (block_start > 0) { mi_memzero_aligned(page, sizeof(*page)); }
    }
  }
  #endif

  // set the guard pages
  mi_subproc_t* const subproc = mi_theap_subproc(theap);
  if (memid->guard_front) { _mi_os_secure_guard_page_set_at(subproc, slice_start, *memid); }
  if (memid->guard_end)   { _mi_os_secure_guard_page_set_at(subproc, slice_start + page_end, *memid); }
}

// With memory tagging, give the page meta data a fresh random tag. Page meta data in front of the slices
// has the opposite tag class of the first block, the separate arena page meta entries have a tag that differs
// from the adjacent entries (see `arena.h:mi_arena_page_meta_memtag`), and the separate page meta data of pages allocated
// directly from the OS (which is not adjacent to other page meta data) gets a tag from the full range.
// Free arena memory is untagged, but the separate arena page meta entries always have a random tag (see `arena.c:mi_arena_memtag_init`).
// (note: aligned page meta data is never used with memory tagging as that always uses `MI_FREE_IS_CHECKED`)
static mi_page_t* mi_page_meta_memtag(mi_page_t* page, uint8_t* slice_start, mi_memid_t memid) {
  if (!mi_memtag_use_for_meta(memid.is_memtag)) return page;
  if (mi_addr(page) == mi_addr(slice_start)) {
    return mi_page_meta_bounded(mi_memtag_meta_init_class(page, sizeof(mi_page_t), 1));
  }
  mi_arena_t* const arena = mi_memid_arena(memid);
  if (arena != NULL && arena->pages_meta != NULL) {
    mi_assert_internal(mi_addr(page) == mi_addr(mi_arena_page_meta_at_slice(arena, memid.mem.arena.slice_index)));
    return mi_page_meta_bounded(mi_arena_page_meta_memtag(arena, page));
  }
  #if MI_PAGE_META_IS_ALIGNED
  return mi_page_meta_bounded(mi_memtag_meta_init_class(page, sizeof(mi_page_t), mi_page_meta_memtag_idx(page)));  // alternate the tag class in aligned arrays
  #else
  return mi_page_meta_bounded(mi_memtag_meta_init_separate(page, sizeof(mi_page_t), mi_page_meta_memtag_idx(page), NULL));
  #endif
}

// Is this page meta data an entry of the separate arena page meta data (`arena->pages_meta`)?
static bool mi_page_meta_is_arena_entry(mi_page_t* page) {
  #if MI_PAGE_META_IS_ALIGNED
  MI_UNUSED(page); return false;
  #else
  return (page->memid.memkind == MI_MEM_ARENA && mi_page_meta_is_separated(page));
  #endif
}

// After freeing a page its meta data cannot be accessed anymore: separate arena page meta entries get a fresh
// random tag while other page meta data is untagged again (as free arena memory).
static void mi_page_meta_memtag_done(mi_page_t* page, mi_memid_t memid) {
  if (!mi_memtag_use_for_meta(memid.is_memtag)) return;
  if (mi_page_meta_is_arena_entry(page)) { mi_arena_page_meta_memtag(mi_memid_arena(memid), page); }
                                    else { mi_memtag_clear(page, sizeof(mi_page_t)); }
}

// Initialize the fields of the page meta data.
static void mi_arenas_page_init_meta(mi_theap_t* theap, mi_page_t* page, uint8_t* slice_start, size_t block_start, size_t block_size,
                                     size_t reserved, size_t commit_size, mi_memid_t memid)
{
  mi_assert_internal(reserved > 0 && reserved <= UINT16_MAX);

  // initialize the page start
  uint8_t* const start = slice_start + block_start;
  mi_assert_internal(mi_addr(start) > mi_addr(page));
  page->page_start = (uint8_t*)mi_ptr_bounded_data(start, reserved * block_size);  // all blocks are derived from this

  // initialize page meta-data
  page->reserved = (uint16_t)reserved;
  page->block_size = block_size;
  #if MI_FREE_IS_CHECKED
  // exact for offsets and block sizes below 2^32 (see `alloc/free.c:mi_page_block_index`)
  mi_assert_internal(reserved==1 || (uint64_t)reserved * block_size <= UINT32_MAX);
  page->block_recip = (reserved > 1 ? (UINT64_MAX / block_size) + 1 : 1);
  #endif
  page->memid = memid;
  page->free_is_zero = memid.initially_zero;
  page->slice_pcommitted = (uint16_t)(commit_size / _mi_os_page_size());

  page->heap = mi_theap_heap(theap);
  #if MI_DEBUG>0
  mi_atomic_store_relaxed(&page->debug_fill_preserve, mi_atomic_load_relaxed(&page->heap->debug_fill_preserve));
  #endif
  if (block_size <= MI_LARGE_MAX_OBJ_SIZE && mi_memtag_use_for(memid.is_memtag, block_size)) {  // huge blocks are not tagged (but have an end guard page in secure mode); over-aligned singletons up to this size are tagged
    mi_page_flags_set(page, true, MI_PAGE_IS_MEMTAGGED);
  }
  mi_page_set_theap(page,theap);
  // mi_assert_internal(mi_page_theap(page) == _mi_heap_theap_peek(page->heap))
}

#if MI_PAGE_META_IS_ALIGNED
// Point the aligned page meta entries of the slices to the actual page meta data.
static void mi_arenas_page_link_aligned_meta(mi_page_t* page, uint8_t* slice_start, size_t slice_count, size_t max_page_meta_count)
{
  mi_page_t* const page_meta = mi_aligned_ptr_page0(slice_start);
  mi_atomic_store_ptr_release(mi_page_t,&mi_page_meta_load(page_meta)->self,page);
  if (slice_count > 1) {
    // at least two for large singleton blocks as guard pages can have a large offset beyond a single slice
    // (note: the entries of adjacent slices are not adjacent, see `mi_page_meta_aligned_index`)
    for(size_t i = 1; i < max_page_meta_count; i++) {
      mi_page_t* const meta = mi_page_meta_load(mi_aligned_ptr_page0(slice_start + _mi_size_of_slices(i)));
      mi_assert_internal(meta->block_size == 0);
      mi_atomic_store_ptr_release(mi_page_t,&meta->self,page);
    }
  }
  #if MI_DEBUG>1
  mi_page_t* pstart = mi_page_meta_load(mi_aligned_ptr_page0(slice_start));
  mi_assert_internal(mi_atomic_load_ptr_acquire(mi_page_t,&pstart->self)==page);
  if (page->reserved>1) {
    mi_page_t* pend = mi_page_meta_load(mi_aligned_ptr_page0(slice_start + (slice_count*MI_ARENA_SLICE_SIZE) - 1));
    mi_assert_internal(mi_atomic_load_ptr_acquire(mi_page_t,&pend->self)==page);
  }
  #endif
}
#endif

// Register a fresh page in its arena pages and the page map, and claim ownership.
// On failure the page is freed.
static bool mi_arenas_page_register(mi_theap_t* theap, mi_page_t* page, mi_arena_pages_t* arena_pages)
{
  // register in the arena_pages
  if (arena_pages!=NULL) {
    mi_assert_internal(page->memid.memkind == MI_MEM_ARENA);
    mi_bitmap_set(arena_pages->pages, page->memid.mem.arena.slice_index);
  }

  // and own it
  mi_page_claim_ownership(page);

  // register in the page map
  if mi_unlikely(!_mi_page_map_register(page)) {
    _mi_arenas_page_free_prim(page);
    return false;
  }

  // stats
  mi_theap_stat_increase(theap, pages, 1);
  mi_theap_stat_increase(theap, page_bins[_mi_page_stats_bin(page)], 1);
  return true;
}

// Allocate a fresh page with the given guard pages (see `mi_theap_page_guards`)
static mi_page_t* mi_arenas_page_alloc_fresh(mi_theap_t* theap, size_t slice_count, size_t block_size, size_t block_alignment, bool commit, mi_page_guards_t guards)
{
  const bool os_align           = (block_alignment > MI_PAGE_MAX_OVERALLOC_ALIGN);
  const bool singleton          = (os_align || block_size > MI_LARGE_MAX_OBJ_SIZE);
  const size_t max_page_meta_count = (singleton && slice_count > 2 ? 2 : slice_count);
  const bool allow_large        = (guards.front == 0 && guards.end == 0);  // large OS pages cannot be protected

  // allocate the slices (always committed with a front guard page, see `mi_arenas_page_commit_first_block`)
  mi_memid_t memid              = _mi_memid_none();
  mi_arena_pages_t* arena_pages = NULL;
  mi_page_t* page_meta          = NULL;
  uint8_t* const slice_start    = mi_arenas_page_alloc_fresh_area(theap,slice_count,max_page_meta_count,block_alignment,os_align,commit || guards.front > 0,allow_large,&memid,&arena_pages,&page_meta);
  if (!slice_start) return NULL;
  // in a torn forked child, the free slices may still have a guard page of a page (or guarded block) that was freed
  // during the fork (as the fork may not copy the memory protection atomically with the memory contents)
  #if MI_SECURE >= 1 || MI_GUARDED
  if (memid.memkind == MI_MEM_ARENA && !memid.is_pinned && _mi_fork_is_torn()) {
    _mi_os_unguard(slice_start, _mi_size_of_slices(slice_count), true /* make accessible */);
  }
  #endif

  // determine the page meta data, the offset of the first block, and the end of the page
  size_t block_start            = 0;
  mi_page_t* page               = mi_page_meta_select(slice_start, slice_count, page_meta, block_size, os_align, &guards, &block_start);
  const size_t alloc_size       = _mi_size_of_slices(slice_count);
  const size_t reserved         = (os_align ? 1 : (alloc_size - guards.end - block_start) / block_size);
  const size_t page_end         = (guards.end > 0 ? mi_align_up(block_start + reserved*block_size, _mi_os_page_size()) : alloc_size);  // the end guard is right after the blocks
  mi_assert_internal(page_end + guards.end <= alloc_size);
  memid.guard_front = (guards.front > 0);
  memid.guard_end   = (guards.end > 0);
  mi_assert_internal(block_size < MI_MAX_ALIGN_SIZE || block_start % MI_MAX_ALIGN_SIZE == 0);
  // all blocks must be naturally aligned (as relied upon by `mi_malloc_aligned`)
  mi_assert_internal(block_size % mi_natural_alignment_of_size(block_size) == 0);
  mi_assert_internal(mi_is_aligned(slice_start + block_start, mi_natural_alignment_of_size(block_size)));
  // on CHERI, all blocks must have exactly representable bounds
  mi_assert_internal(mi_bounds_size(block_size) == block_size && mi_is_aligned(slice_start + block_start, mi_bounds_align(block_size)));

  // commit the first block (if needed)
  size_t commit_size = 0;
  if (!mi_arenas_page_commit_first_block(theap, memid, slice_start, block_start, block_size, page_end, &commit_size)) {
    _mi_arenas_free(mi_theap_subproc(theap), slice_start, alloc_size, memid);
    return NULL;
  }
  mi_assert_internal((commit && commit_size==0) || (!commit && (commit_size <= UINT16_MAX * _mi_os_page_size())));

  // initialize the page memory and meta data
  mi_arenas_page_init_memory(theap, page, slice_start, slice_count, block_start, page_end, &memid);
  page = mi_page_meta_memtag(page, slice_start, memid);
  mi_arenas_page_init_meta(theap, page, slice_start, block_start, block_size, reserved, commit_size, memid);
  #if MI_PAGE_META_IS_ALIGNED
  mi_arenas_page_link_aligned_meta(page, slice_start, slice_count, max_page_meta_count);
  #endif
  mi_assert_internal(page->free==NULL);
  mi_assert_internal((mi_addr(page) != mi_addr(slice_start)) == mi_page_meta_is_separated(page));
  mi_assert_internal(mi_page_slice_start(page) == slice_start);
  mi_assert_internal(mi_page_slice_offset_of(page, mi_page_size(page)) <= page_end);
  mi_assert_internal(!memid.guard_end || mi_page_guard_end_offset(page) == page_end);

  // now we can register the page (and use `_mi_arenas_page_free_prim` on error)
  if (!mi_arenas_page_register(theap, page, arena_pages)) return NULL;

  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page),MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_block_size(page) == block_size);
  #if MI_PAGE_META_SMALL_IS_ALIGNED
  // `mi_free_small` finds the page meta data by aligning a block pointer down to `MI_SMALL_PAGE_SIZE`
  if (mi_page_is_small(page)) {
    mi_assert_internal(mi_addr(page) == mi_addr(slice_start) && mi_is_aligned(page, MI_SMALL_PAGE_SIZE));
    mi_assert_internal(mi_addr(mi_page_start(page)) + mi_page_size(page) <= mi_addr(page) + MI_SMALL_PAGE_SIZE);
  }
  #endif
  // mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(mi_page_is_owned(page));
  return page;
}





/* -----------------------------------------------------------
  Allocate page (`_mi_arenas_page_alloc`)
----------------------------------------------------------- */

// Allocate a regular small/medium/large page.
static mi_page_t* mi_arenas_page_regular_alloc(mi_theap_t* theap, size_t slice_count, size_t block_size)
{
  // 1. look for an abandoned page
  const mi_page_guards_t guards = mi_theap_page_guards(theap, _mi_size_of_slices(slice_count), false, false);
  mi_page_t* page = _mi_arenas_page_try_find_abandoned(theap, slice_count, block_size, guards);
  if (page != NULL) {
    return page;  // return as abandoned
  }

  // 2. find a free block, potentially allocating a new arena
  const long commit_on_demand = mi_option_get(mi_option_page_commit_on_demand);
  const bool commit = (mi_page_min_commit_size() >= slice_count * MI_ARENA_SLICE_SIZE ||           // always commit small pages
                       (slice_count >= _mi_slice_count_of_size(UINT16_MAX * _mi_os_page_size())) ||   // always commit pages too large to hold a 32-bit slice_committed
                        (commit_on_demand == 2 && _mi_os_has_overcommit()) || (commit_on_demand == 0));
  page = mi_arenas_page_alloc_fresh(theap, slice_count, block_size, 1, commit, guards);
  if (page == NULL) return NULL;

  mi_assert_internal(page->memid.memkind != MI_MEM_ARENA || page->memid.mem.arena.slice_count == slice_count);
  if (!_mi_page_init(theap, page)) {
    _mi_arenas_page_free_direct(page,theap);
    return NULL;
  }

  return page;
}

// Allocate a page containing one block (very large, or with large alignment)
static mi_page_t* mi_arenas_page_singleton_alloc(mi_theap_t* theap, size_t block_size, size_t block_alignment)
{
  const bool os_align = (block_alignment > MI_PAGE_MAX_OVERALLOC_ALIGN);
  #if MI_PAGE_META_OS_IS_SEPARATED
  const size_t info_size = 0;
  #else
  const size_t info_size = (os_align ? MI_PAGE_ALIGN : mi_align_up(mi_page_info_size(), mi_bounds_align(block_size)));  // see `mi_page_meta_in_front_block_start`
  #endif
  // room for the cache offset of the block (committed but not touched)
  const size_t cache_offset_size = (!os_align && mi_page_block_can_use_large_cache_offset(block_size) ? MI_PAGE_LARGE_CACHE_OFFSET_MAX : 0);
  // determine the guard pages based on the page size without guard pages
  const size_t size = info_size + cache_offset_size + block_size;
  const mi_page_guards_t guards = mi_theap_page_guards(theap, _mi_size_of_slices(_mi_slice_count_of_size(size)), true, os_align);
  // with memory tagging, a singleton without cache offset or end guard page may fill its slices exactly and share its
  // tag class with the first block of an adjacent page; reserve a tag 0 unit at the end (see `mi_page_memtag_parity`)
  #if MI_MTE
  const size_t memtag_slack = (cache_offset_size == 0 && guards.end == 0 ? MI_MEMTAG_UNIT_SIZE : 0);
  #else
  const size_t memtag_slack = 0;
  #endif
  const size_t slice_count = _mi_slice_count_of_size(mi_page_guard_front_block_start(guards.front, block_size) + size + memtag_slack + guards.end);

  mi_page_t* page = mi_arenas_page_alloc_fresh(theap, slice_count, block_size, block_alignment, true /* commit singletons always */, guards);
  if (page == NULL) return NULL;

  mi_assert(page->reserved == 1);
  if (!_mi_page_init(theap, page)) {
    _mi_arenas_page_free_direct(page,theap);
    return NULL;
  }

  return page;
}


mi_page_t* _mi_arenas_page_alloc(mi_theap_t* theap, size_t block_size, size_t block_alignment) {
  mi_page_t* page;
  // semi static assert: ensure that all non-singleton block size bins are covered.
  mi_assert(_mi_bin(MI_LARGE_MAX_OBJ_SIZE)  < MI_ARENA_BIN_COUNT);
  if mi_unlikely(block_alignment > MI_PAGE_MAX_OVERALLOC_ALIGN) {
    mi_assert_internal(mi_is_power_of_two(block_alignment));
    page = mi_arenas_page_singleton_alloc(theap, block_size, block_alignment);
  }
  else if (block_size <= MI_SMALL_MAX_OBJ_SIZE) {
    page = mi_arenas_page_regular_alloc(theap, _mi_slice_count_of_size(MI_SMALL_PAGE_SIZE), block_size);
  }
  else if (mi_block_size_is_medium(block_size)) {
    page = mi_arenas_page_regular_alloc(theap, _mi_slice_count_of_size(MI_MEDIUM_PAGE_SIZE), block_size);
  }
  #if MI_ENABLE_LARGE_PAGES
  else if (block_size <= MI_LARGE_MAX_OBJ_SIZE) {
    page = mi_arenas_page_regular_alloc(theap, _mi_slice_count_of_size(MI_LARGE_PAGE_SIZE), block_size);
  }
  #endif
  else {
    page = mi_arenas_page_singleton_alloc(theap, block_size, block_alignment);
  }
  if mi_unlikely(page == NULL) {
    return NULL;
  }
  // mi_assert_internal(page == NULL || _mi_page_segment(page)->subproc == tld->subproc);
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(block_alignment <= MI_PAGE_MAX_OVERALLOC_ALIGN || mi_is_aligned(mi_page_start(page), block_alignment));

  return page;
}


/* -----------------------------------------------------------
  Free a page
----------------------------------------------------------- */

static size_t mi_page_full_size(mi_page_t* page) {
  if (page->memid.memkind == MI_MEM_ARENA) {
    return page->memid.mem.arena.slice_count * MI_ARENA_SLICE_SIZE;
  }
  else if (mi_memid_is_os(page->memid)) {
    // note: the page meta data may be in front of the slices (see `mi_arenas_page_alloc_from_os`)
    uint8_t* const slice_start = mi_page_slice_start(page);
    mi_assert_internal((uint8_t*)page->memid.mem.os.base <= slice_start);
    const size_t presize = (size_t)(slice_start - (uint8_t*)page->memid.mem.os.base);
    const size_t os_size = mi_memid_os_size(page->memid);
    mi_assert_internal(os_size >= presize);
    return (presize > os_size ? 0 : os_size - presize);
  }
  else {
    return 0;
  }
}

void _mi_arenas_page_free_prim(mi_page_t* page) {
  mi_assert_internal(_mi_page_qsbr_is_clear(page));
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(page->next==NULL && page->prev==NULL);

  mi_subproc_t* const subproc = mi_page_subproc(page);  // read before the page leaves its heap (as the heap may be deleted concurrently)
  mi_arena_t* arena = NULL;
  mi_arena_pages_t* arena_pages = NULL;
  size_t slice_index = 0;
  size_t slice_count = 0;
  if (page->memid.memkind == MI_MEM_ARENA) {
    arena = _mi_page_arena_pages(page, &slice_index, &slice_count, &arena_pages);
    mi_assert_internal(arena_pages!=NULL);
    mi_assert_internal(arena->subproc == subproc);
    #if MI_DEBUG>1
    if (!mi_page_is_full(page)) {
      size_t bin = _mi_bin(mi_page_block_size(page));
      mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
      mi_assert_internal(mi_page_slice_committed(page) > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
      mi_assert_internal(bin >= MI_ARENA_BIN_COUNT || mi_bitmap_chunk_count(arena_pages->pages_abandoned[bin]) == 0 || mi_bitmap_is_clearN(arena_pages->pages_abandoned[bin], slice_index, 1));
      // note: we cannot check for `!mi_page_is_abandoned_and_mapped` since that may
      // be (temporarily) not true if the free happens while trying to reclaim
      // see `mi_arena_try_claim_abandoned`
    }
    #endif
    // first remove the page from its heap; this waits if moving heap pages temporarily
    // cleared the bit to safely claim the page (see `mi_heap_move_page_at`)
    mi_bitmap_clear_once_set(arena->subproc, arena_pages->pages, slice_index);
  }

  // unregister page
  _mi_page_map_unregister(page);

  // reset the memory tags of the blocks so free memory is untagged again
  if (mi_page_is_memtag(page)) {
    mi_memtag_clear(mi_page_start(page), mi_page_capacity_size(page));
  }

  // reset the guard pages
  // we must do this since we may later allocate large spans over this page and cannot have a guard page in between
  // (for a page committed on demand the end guard page stays decommitted and its commit bit is clear)
  uint8_t* const slice_start = mi_page_slice_start(page);
  if (page->memid.guard_front) { _mi_os_secure_guard_page_reset_at(subproc, slice_start, page->memid); }
  if (page->memid.guard_end)   { _mi_os_secure_guard_page_reset_at(subproc, slice_start + mi_page_guard_end_offset(page), page->memid); }

  // and free
  mi_memid_t memid = page->memid;
  const size_t slice_committed = mi_page_slice_committed(page);
  if (arena != NULL) {
    if (slice_committed > 0) {
      // if committed on-demand, set the commit bits to account commit properly
      mi_assert_internal(mi_page_full_size(page) >= slice_committed);
      const size_t total_slices = slice_committed / MI_ARENA_SLICE_SIZE;  // conservative
      //mi_assert_internal(mi_bitmap_is_clearN(arena->slices_committed, slice_index, total_slices));
      mi_assert_internal(slice_count >= total_slices);
      if (total_slices > 0) {
        mi_bitmap_setN(arena->slices_committed, slice_index, total_slices, NULL);
      }
      // any left over?
      const size_t extra = slice_committed % MI_ARENA_SLICE_SIZE;
      if (extra > 0) {
        // pretend it was decommitted already
        mi_subproc_stat_decrease(arena->subproc, committed, extra);
      }
    }
    else {
      mi_assert_internal(mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
    }
  }
  else {
    mi_heap_stat_decrease(page->heap, pages_os_allocated, 1);
    mi_atomic_sub_acq_rel(&page->heap->os_page_count, 1);  // last access to the heap (see `mi_heap_move_pages`)
    if (slice_committed > 0) { mi_memid_os_set_committed(&memid, slice_committed); }  // committed on-demand
  }
  if (mi_page_meta_is_separated(page)) { page->block_size = 0; }  // for assertion checking
  const size_t full_size = mi_page_full_size(page);
  mi_page_meta_memtag_done(page, memid);  // the page meta data cannot be accessed anymore
  _mi_arenas_free(subproc, slice_start, full_size, memid);
}

// Physically release an empty page whose QSBR state has already been cleared.
void _mi_arenas_page_free_direct(mi_page_t* page, mi_theap_t* current_theapx) {
  mi_assert_internal(_mi_page_qsbr_is_clear(page));
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(page->next==NULL && page->prev==NULL);
  mi_assert_internal(mi_theap_matches_thread(current_theapx));

  mi_heap_t* const heap = mi_page_heap(page);
  mi_theapx_stat_decrease(heap, current_theapx, page_bins[_mi_page_stats_bin(page)], 1);
  mi_theapx_stat_decrease(heap, current_theapx, pages, 1);
  _mi_page_free_collect(page,false);  // update used count for cross-thread free's
  _mi_page_update_stats(page);        // and update the stats
  _mi_arenas_page_free_prim(page);
}

// Free an empty page that was removed from the abandoned registries, or retain it if it is delayed by QSBR:
// a delayed singleton goes to the heap QSBR delayed list, and other pages are abandoned (to be reclaimed or collected later).
// Returns true if the page was freed.
bool _mi_arenas_page_free(mi_page_t* page, mi_theap_t* current_theapx) {
  mi_assert_internal(mi_page_is_owned(page) && mi_page_all_free(page));
  mi_assert_internal(mi_page_is_abandoned(page) && !mi_page_is_abandoned_mapped(page));
  mi_assert_internal(page->next == NULL && page->prev == NULL);
  _mi_page_check_free_lists(page);
  if (_mi_page_qsbr_should_delay(page)) {
    #if MI_QSBR
    if (mi_page_is_singleton(page)) {
      _mi_heap_qsbr_delayed_enqueue(page);
      return false;
    }
    #endif
    _mi_arenas_page_abandon(page, current_theapx);
    return false;
  }
  _mi_page_qsbr_clear(page);
  _mi_arenas_page_free_direct(page, current_theapx);
  return true;
}
