/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_ALLOC_PAGE_H
#define MI_ALLOC_PAGE_H

#include "../types.h"
#include "../prim/prim-mte.h"  // mi_ptr_memtag_meta, mi_memtag_exclude_base, MI_ZERO_ON_FREE_MAX
#include "../prim/os.h"
#include "../heap/heap.h"
#include "../heap/page-map.h"
#include "../prim/prim-pac.h"
#include "../arena/arena.h"
#include "../track.h"

mi_decl_restrict void* _mi_malloc_generic(mi_theap_t* theap, size_t size, size_t zero_huge_alignment, mi_page_t** ppage) mi_attr_noexcept mi_attr_malloc;
mi_decl_restrict void* _mi_malloc_generic_no_sample(mi_theap_t* theap, size_t size, bool zero, mi_page_t** ppage) mi_attr_noexcept mi_attr_malloc;

void          _mi_page_retire(mi_page_t* page) mi_attr_noexcept;       // free the page if there are no other pages with many free blocks
void          _mi_page_unfull(mi_page_t* page);
bool          _mi_page_free(mi_page_t* page, mi_page_queue_t* pq);     // false if page release is delayed
void          _mi_page_abandon(mi_page_t* page, mi_page_queue_t* pq, bool owner_done);  // abandon the page, to be picked up by another thread...
void          _mi_deferred_free(mi_theap_t* theap, bool force);
void          _mi_page_check_free_lists(const mi_page_t* page);   // check an empty page before it is freed (at `MI_SECURE>=1`)
bool          _mi_page_free_collect(mi_page_t* page, bool force);  // returns `true` if cross-thread free'd blocks were collected
mi_block_t*   _mi_page_free_collect_partly(mi_page_t* page, mi_block_t* head);
mi_decl_nodiscard bool _mi_page_init(mi_theap_t* theap, mi_page_t* page);
void          _mi_page_update_stats(mi_page_t* page);

void          _mi_theap_collect_retired(mi_theap_t* theap, bool force);
void          _mi_theap_collect_huge_pages(mi_theap_t* theap);
bool          _mi_theap_page_reclaim(mi_theap_t* theap, mi_page_t* page);  // true if an empty page was freed
#if MI_DEBUG>1
bool          _mi_page_is_valid(mi_page_t* page);
#endif

// in page-queue.c
bool          _mi_page_queue_is_valid(mi_theap_t* theap, const mi_page_queue_t* pq);
size_t        _mi_page_stats_bin(const mi_page_t* page); // for stats
size_t        _mi_bin_size(size_t bin);                  // for stats
size_t        _mi_bin(size_t size);                      // for stats
size_t        _mi_block_size_of(size_t size);            // block size (including padding) for an allocation of `size` bytes


static inline mi_page_queue_t* mi_page_queue(const mi_theap_t* theap, size_t size) {
  mi_page_queue_t* const pq = &((mi_theap_t*)theap)->pages[_mi_bin(size)];
  if (size <= MI_LARGE_MAX_OBJ_SIZE) { mi_assert_internal(pq->block_size <= MI_LARGE_MAX_OBJ_SIZE); }
  return pq;
}

// Get the block size of a page
static inline size_t mi_page_block_size(const mi_page_t* page) {
  mi_assert_internal(page->block_size > 0);
  return page->block_size;
}

// Page start
static inline uint8_t* mi_page_start(const mi_page_t* page) {
  return page->page_start;
}

static inline size_t mi_page_size(const mi_page_t* page) {
  return mi_page_block_size(page) * page->reserved;
}

// Byte size of the committed blocks (`capacity * block_size`); can be read by non-owning threads if `MI_FREE_IS_CHECKED`
static inline size_t mi_page_capacity_size(const mi_page_t* page) {
  #if MI_FREE_IS_CHECKED
  return mi_atomic_load_relaxed(&page->capacity_size);
  #else
  return (size_t)page->capacity * mi_page_block_size(page);
  #endif
}

#if MI_FREE_IS_CHECKED
// Is the byte offset `ofs` (below 2^32) at the start of a block? `ofs` is a multiple of `block_size` iff
// `ofs * ceil(2^64/block_size) mod 2^64 < ceil(2^64/block_size)` (Lemire, Kaser, and Kurz, "Faster remainder by direct computation", 2019).
// With a single block in the page, `block_recip == 1` and this checks `ofs == 0`.
static inline bool mi_page_is_block_offset(const mi_page_t* page, size_t ofs) {
  return ((uint64_t)ofs * page->block_recip < page->block_recip);
}
#endif

static inline uint8_t* mi_page_area(const mi_page_t* page, size_t* size) {
  if (size) { *size = mi_page_size(page); }
  return mi_page_start(page);
}

static inline size_t mi_page_info_size(void) {
  return mi_align_up(sizeof(mi_page_t), MI_MAX_ALIGN_SIZE);
}

// Can the first block in a page with blocks of size `bsize` have a large cache offset? (see `MI_PAGE_LARGE_CACHE_OFFSET`; smaller blocks use a small cache offset)
static inline bool mi_block_size_has_large_cache_offset(size_t bsize) {
  #if MI_PAGE_LARGE_CACHE_OFFSET
  return (bsize > MI_SMALL_MAX_OBJ_SIZE);
  #else
  return false;
  #endif
}

// Is a (non-small) block of size `bsize` allocated in a medium page?
// Blocks of 1/8th of a medium page (64 KiB) leave no space for a cache offset and use a large page instead.
static inline bool mi_block_size_is_medium(size_t bsize) {
  #if MI_ENABLE_LARGE_PAGES
  if (mi_block_size_has_large_cache_offset(bsize) && bsize == MI_MEDIUM_PAGE_SIZE/8) return false;
  #endif
  return (bsize <= MI_MEDIUM_MAX_OBJ_SIZE);
}

// The alignment that every block of size `bsize` is guaranteed to have in any page (or 0 if `bsize==0`).
// This is guaranteed by the block start (see `arena/arena-page-alloc.c:mi_page_meta_in_front_block_start` and `mi_page_meta_select`),
// and used by `mi_malloc_aligned` to avoid over-allocation for naturally aligned blocks.
static inline size_t mi_natural_alignment_of_size(size_t bsize) {
  const size_t lowbit = bsize & (~bsize + 1);  // largest power of 2 that divides `bsize`
  if (bsize <= MI_SMALL_SIZE_MAX) return lowbit;                                            // small blocks start at a multiple of their size
  if (mi_is_power_of_two(bsize) && bsize <= MI_PAGE_MAX_START_BLOCK_ALIGN2) return bsize;   // power-of-two blocks up to 4KiB
  if ((bsize % MI_PAGE_OSPAGE_BLOCK_ALIGN2) == 0) {                                         // multiples of 4KiB are 4KiB aligned (or cache offset unit aligned)
    return (mi_block_size_has_large_cache_offset(bsize) ? MI_PAGE_LARGE_CACHE_OFFSET_UNIT : MI_PAGE_OSPAGE_BLOCK_ALIGN2);
  }
  return (lowbit < MI_MAX_ALIGN_SIZE ? lowbit : MI_MAX_ALIGN_SIZE);                         // otherwise blocks start at least `MI_MAX_ALIGN_SIZE` aligned
}

static inline bool mi_page_contains_address(const mi_page_t* page, const void* p) {
  size_t psize;
  const mi_addr_t start = mi_addr(mi_page_area(page, &psize));
  return (start <= mi_addr(p) && mi_addr(p) < start + psize);
}

static inline bool mi_page_is_in_arena(const mi_page_t* page) {
  return (page->memid.memkind == MI_MEM_ARENA);
}

static inline bool mi_page_is_singleton(const mi_page_t* page) {
  return (page->reserved == 1);
}

// Get the usable block size of a page without fixed padding.
// This may still include internal padding due to alignment and rounding up size classes.
static inline size_t mi_page_usable_block_size(const mi_page_t* page) {
  return mi_page_block_size(page) - MI_PADDING_SIZE;
}

static inline bool mi_page_meta_is_separated(const mi_page_t* page) {
  #if MI_PAGE_META_IS_ALIGNED
    #if MI_PAGE_META_SMALL_IS_ALIGNED
    return (mi_addr(page) != mi_align_down(mi_addr(mi_page_start(page)), MI_ARENA_SLICE_ALIGN));
    #else
    MI_UNUSED_RELEASE(page);
    mi_assert_internal(mi_addr(page) != mi_align_down(mi_addr(mi_page_start(page)), MI_ARENA_SLICE_ALIGN));
    return true;
    #endif
  #elif MI_PAGE_META_OS_IS_SEPARATED && !MI_PAGE_META_SMALL_IS_ALIGNED
  MI_UNUSED_RELEASE(page);
  mi_assert_internal(mi_addr(page) != mi_align_down(mi_addr(mi_page_start(page)), MI_ARENA_SLICE_ALIGN));
  return true;
  #elif MI_PAGE_META_IS_SEPARATED
  // usually separated but can still be in front for direct OS allocations (due to size or alignment) or due to MI_PAGE_META_SMALL_IS_ALIGNED
  return (page->memid.memkind == MI_MEM_ARENA && mi_addr(page) != mi_align_down(mi_addr(mi_page_start(page)), MI_ARENA_SLICE_ALIGN));
  #else
  MI_UNUSED(page);
  return false;
  #endif
}

static inline uint8_t* mi_page_slice_start(const mi_page_t* page) {
  #if MI_CHERI
  // derive from the arena or OS memory as both the page meta data and the page start are bounded
  if (page->memid.memkind == MI_MEM_ARENA) {
    return mi_arena_start(mi_memid_arena(page->memid)) + ((size_t)page->memid.mem.arena.slice_index * MI_ARENA_SLICE_SIZE);
  }
  else {
    void* const slice_start = (mi_page_meta_is_separated(page) ? mi_align_down_ptr(mi_page_start(page), MI_ARENA_SLICE_ALIGN) : (void*)page);
    return (uint8_t*)mi_ptr_rederive(page->memid.mem.os.base, slice_start);
  }
  #else
  if (mi_page_meta_is_separated(page)) {
    // page meta info is at a separate location (at `arena->pages`)
    return (uint8_t*)mi_align_down_ptr(mi_page_start(page), MI_ARENA_SLICE_ALIGN);
  }
  else {
    // page meta info is at the start of the page slices (without its memory tag)
    return (uint8_t*)mi_ptr_memtag_meta(page);
  }
  #endif
}

// Re-derive `p` in the page memory with the authority to change its protection
static inline void* mi_page_vmem_ptr(const mi_page_t* page, void* p) {
  #if MI_CHERI
  return mi_ptr_rederive(mi_page_slice_start(page), p);
  #else
  MI_UNUSED(page);
  return p;
  #endif
}

// This gives the offset relative to the start slice of a page.
static inline size_t mi_page_slice_offset_of(const mi_page_t* page, size_t offset_relative_to_page_start) {
  return (mi_page_start(page) - mi_page_slice_start(page)) + offset_relative_to_page_start;
}

// The end of the blocks relative to the slice start rounded up to the OS page size.
// This is where the end guard page is if `page->memid.guard_end` is set.
static inline size_t mi_page_guard_end_offset(const mi_page_t* page) {
  return mi_align_up(mi_page_slice_offset_of(page, mi_page_size(page)), _mi_os_page_size());
}

// How much of the page is committed relative to the slice start? (or 0 if fully committed already)
static inline size_t mi_page_slice_committed(const mi_page_t* page) {
  return ((size_t)page->slice_pcommitted * _mi_os_page_size());
}

// Currently committed part of a page
static inline size_t mi_page_committed(const mi_page_t* page) {
  const size_t slice_committed = mi_page_slice_committed(page);
  return (slice_committed == 0 ? mi_page_size(page) : slice_committed - mi_page_slice_offset_of(page,0));
}

static inline size_t mi_page_used(const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  return mi_xused_used_count(page->xused);
}

static inline void mi_page_used_reset(mi_page_t* page) {
  page->xused = mi_xused_used_reset(page->xused);
}

static inline size_t mi_page_alloc_count(const mi_page_t* page) {
  return mi_xused_alloc_count(page->xused);
}

static inline size_t mi_page_last_used(const mi_page_t* page) {
  #if MI_SIZE_SIZE >= 8
  return (page->xused.used_alloc >> 32) & 0xFFFF;
  #else
  return page->xlast_used;
  #endif
}

static inline size_t mi_page_last_alloc(const mi_page_t* page) {
  #if MI_SIZE_SIZE >= 8
  return (page->xused.used_alloc >> 48) & 0xFFFF;
  #else
  return page->xlast_alloc;
  #endif
}


// are all blocks in a page freed?
// note: needs up-to-date used count, (as the `xthread_free` list may not be empty). see `_mi_page_collect_free`.
static inline bool mi_page_all_free(const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  return (mi_page_used(page)==0);
}

// are there immediately available blocks, i.e. blocks available on the free list.
static inline bool mi_page_immediate_available(const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  return (page->free != NULL);
}


// is the page not yet used up to its reserved space?
static inline bool mi_page_is_expandable(const mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_internal(page->capacity <= page->reserved);
  return (page->capacity < page->reserved);
}


static inline bool mi_page_is_full(const mi_page_t* page) {
  const bool full = (page->reserved == mi_page_used(page));
  mi_assert_internal(!full || page->free == NULL);
  return full;
}

// is more than (1 - 1/n)'th of a page in use?
static inline bool mi_page_is_used_at_frac(const mi_page_t* page, size_t n) {
  if (page==NULL) return true;
  const size_t frac = page->reserved / n;
  return (page->reserved - mi_page_used(page) <= frac);
}

// is more than 7/8th of a page in use?
static inline bool mi_page_is_mostly_used(const mi_page_t* page) {
  return mi_page_is_used_at_frac(page, 8);
}

// Is reuse of the freed blocks in this page delayed? (see `MI_SECURE_REUSE_DELAY`)
static inline bool mi_page_reuse_is_delayed(const mi_page_t* page) {
  #if (MI_SECURE_REUSE_DELAY > 0)
  const size_t capacity = page->capacity;
  size_t delay = capacity / MI_SECURE_REUSE_DELAY;
  const size_t delay_min = (capacity/2 < MI_SECURE_REUSE_DELAY_MIN ? capacity/2 : MI_SECURE_REUSE_DELAY_MIN);
  if (delay < delay_min) { delay = delay_min; }
  if (delay > MI_SECURE_REUSE_DELAY_MAX) { delay = MI_SECURE_REUSE_DELAY_MAX; }
  return (capacity - mi_page_used(page) < delay);
  #else
  MI_UNUSED(page);
  return false;
  #endif
}

static inline bool mi_page_is_small(const mi_page_t* page) {
  return (page->block_size <= MI_SMALL_MAX_OBJ_SIZE && !mi_page_is_singleton(page));
}

static inline bool mi_page_is_huge(const mi_page_t* page) {
  return (mi_page_is_singleton(page) &&
          (page->block_size > MI_LARGE_MAX_OBJ_SIZE ||
           (mi_memkind_is_os(page->memid.memkind) && page->memid.mem.os.base < (void*)page)));
}

static inline size_t mi_page_min_commit_size(void) {
  const size_t psize = _mi_os_page_size();
  return (MI_PAGE_MIN_COMMIT_SIZE >= psize ? MI_PAGE_MIN_COMMIT_SIZE : psize);
}

//-----------------------------------------------------------
// Page thread id and flags
//-----------------------------------------------------------

// Thread id of thread that owns this page (with flags in the bottom 2 bits)
static inline mi_thread_id_t mi_page_xthread_id(const mi_page_t* page) {
  return mi_atomic_load_relaxed(&((mi_page_t*)page)->xthread_id);
}

// Plain thread id of the thread that owns this page
static inline mi_thread_id_t mi_page_thread_id(const mi_page_t* page) {
  return (mi_page_xthread_id(page) & ~MI_PAGE_FLAG_MASK);
}

static inline mi_page_flags_t mi_page_flags(const mi_page_t* page) {
  return (mi_page_xthread_id(page) & MI_PAGE_FLAG_MASK);
}

static inline bool mi_page_flags_set(mi_page_t* page, bool set, mi_page_flags_t newflag) {
  mi_page_flags_t old;
  if (set) { old = mi_atomic_or_relaxed(&page->xthread_id, newflag); }
      else { old = mi_atomic_and_relaxed(&page->xthread_id, ~newflag); }
  return ((old & newflag) == newflag);
}

static inline bool mi_page_is_in_full(const mi_page_t* page) {
  return ((mi_page_flags(page) & MI_PAGE_IN_FULL_QUEUE) != 0);
}

static inline void mi_page_set_in_full(mi_page_t* page, bool in_full) {
  const bool was_in_full = mi_page_flags_set(page, in_full, MI_PAGE_IN_FULL_QUEUE);
  if (was_in_full != in_full) {
    // optimize: maintain pages_full_size to avoid visiting the full queue (issue #1220)
    mi_theap_t* const theap = page->theap;
    mi_assert_internal(theap!=NULL);
    if (theap != NULL) {
      mi_assert_internal(page->capacity==page->reserved);
      const size_t size = page->reserved * mi_page_block_size(page);
      if (in_full) { theap->pages_full_size += size; }
              else { mi_assert_internal(size <= theap->pages_full_size); theap->pages_full_size -= size; }
    }
  }
}

static inline bool mi_page_has_interior_pointers(const mi_page_t* page) {
  return ((mi_page_flags(page) & MI_PAGE_HAS_INTERIOR_POINTERS) != 0);
}

static inline void mi_page_set_has_interior_pointers(mi_page_t* page, bool has_aligned) {
  mi_page_flags_set(page, has_aligned, MI_PAGE_HAS_INTERIOR_POINTERS);
}

// Are the blocks in this page memory tagged? (always false without `MI_MTE`)
static inline bool mi_page_is_memtag(const mi_page_t* page) {
  return ((mi_page_flags(page) & MI_PAGE_IS_MEMTAGGED) != 0);
}

// The tag class parity of the blocks in a memory tagged page: block `i` has the class of `i + parity` (see `prim-mte.h`).
// Adjacent pages must not share a tag class across their boundary, so a page starts with an even block (or a tag 0 gap)
// and ends with an odd block (or a tag 0 gap). When there is a (tag 0) cache offset gap in front of an odd number of
// blocks, we flip the parity such that the last block is odd. (A page without a front gap and an odd number of blocks
// that fills its slices exactly can only be a singleton (as regular page sizes are a power of two), which gets a tag 0
// unit at the end, see `mi_arenas_page_singleton_alloc`.)
// (note: with page meta data in front of the slices, which happens only for OS-allocated pages, the meta data has the odd class)
static inline size_t mi_page_memtag_parity(const mi_page_t* page) {
  return ((page->reserved & 1) != 0 && mi_page_meta_is_separated(page) && mi_page_start(page) != mi_page_slice_start(page) ? 1 : 0);
}

// The tag exclusion mask for (re)tagging the `block` of size `bsize` in a memory tagged page, and the adjacent blocks in `*prev`
// and `*next` (or NULL) such that the new tag can differ from the tags of the adjacent blocks (see `prim-mte.h:mi_memtag_random_adjacent`).
// Only the first unit of the neighbours is read (as a guarded block may end in a guard page), and only for blocks below the capacity
// (as later blocks are not yet tagged and may not be committed). The first and the last block of the page use the tag class of `idx`
// (as the adjacent memory cannot be read).
// (note: the capacity is read concurrently by a non-owning thread (using the atomic `capacity_size`) but it only grows while the page
//  has a used block; a stale capacity only means the next block is not considered, which is the same as a free just before an extension)
// (note: we branch on the (rare) edge case instead of selecting the mask as this keeps it off the critical path of the retagging)
static inline size_t mi_page_block_memtag_adjacent(const mi_page_t* page, const void* block, size_t bsize, size_t idx, const void** prev, const void** next) {
  const uint8_t* const ublock = (const uint8_t*)mi_ptr_memtag_meta(block);
  const size_t ofs = (size_t)(mi_addr(ublock) - mi_addr(mi_page_start(page)));
  const size_t capacity_size = mi_page_capacity_size(page);
  *prev = (ofs == 0 ? NULL : ublock - bsize);
  *next = (ofs + bsize >= capacity_size ? NULL : ublock + bsize);
  if mi_unlikely(ofs == 0 || ofs + bsize >= (size_t)page->reserved * bsize) {
    return mi_memtag_exclude_base(idx, true);
  }
  return mi_memtag_exclude_base(idx, false);
}

// Are freed blocks in this page zeroed (except for the free list link)? This holds for small blocks that are
// memory tagged or with `MI_SECURE>=3` (always false otherwise, or in debug mode as freed blocks are filled then)
static inline bool mi_page_is_zero_on_free(const mi_page_t* page) {
  #if (MI_DEBUG>0) && !MI_TRACK_ENABLED && !MI_TSAN
  MI_UNUSED(page);
  return false;
  #else
  return ((MI_SECURE>=3 || mi_page_is_memtag(page)) && mi_page_block_size(page) <= MI_ZERO_ON_FREE_MAX);
  #endif
}

#if MI_QSBR
// For qsbr, `qsbr_xthread_id` keeps the thread id of the last owning thread (so it is still available when a full page is abandoned)
// It is cleared when the owning theap abandons all its pages (e.g. on thread termination).
// The lowest bit is set if the page is in the qsbr queue of its theap. It is only written by the owning thread.
#define MI_PAGE_QSBR_IN_QUEUE  MI_ZU(0x01)

static inline mi_thread_id_t mi_page_qsbr_xthread_id(const mi_page_t* page) {
  return mi_atomic_load_relaxed(&((mi_page_t*)page)->qsbr_xthread_id);
}

static inline mi_thread_id_t mi_page_qsbr_thread_id(const mi_page_t* page) {
  return (mi_page_qsbr_xthread_id(page) & ~MI_PAGE_FLAG_MASK);
}

static inline void mi_page_qsbr_set_thread_id(mi_page_t* page, mi_thread_id_t tid) {
  mi_assert_internal((tid & MI_PAGE_FLAG_MASK) == 0);
  const mi_thread_id_t xtid = mi_page_qsbr_xthread_id(page);
  mi_atomic_store_relaxed(&page->qsbr_xthread_id, tid | (xtid & MI_PAGE_QSBR_IN_QUEUE));
}

static inline bool mi_page_qsbr_is_in_queue(const mi_page_t* page) {
  return ((mi_page_qsbr_xthread_id(page) & MI_PAGE_QSBR_IN_QUEUE) != 0);
}

static inline void mi_page_qsbr_set_in_queue(mi_page_t* page, bool in_queue) {
  const mi_thread_id_t xtid = mi_page_qsbr_xthread_id(page);
  mi_atomic_store_relaxed(&page->qsbr_xthread_id, (in_queue ? (xtid | MI_PAGE_QSBR_IN_QUEUE) : (xtid & ~MI_PAGE_QSBR_IN_QUEUE)));
}
#endif

static inline void mi_page_set_theap(mi_page_t* page, mi_theap_t* theap) {
  // mi_assert_internal(!mi_page_is_in_full(page));  // can happen when destroying pages on theap_destroy
  page->theap = theap;
  const mi_thread_id_t tid = (theap == NULL ? MI_THREAD_ID_ABANDONED : theap->tld->thread_id);
  mi_assert_internal((tid & MI_PAGE_FLAG_MASK) == 0);

  // Preserve flags set by concurrent aligned allocations.
  if (theap == NULL) {
    mi_atomic_and_release(&page->xthread_id, MI_PAGE_FLAG_MASK);
    return;
  }
  mi_thread_id_t xtid_old = mi_page_xthread_id(page);
  mi_thread_id_t xtid;
  do {
    xtid = tid | (xtid_old & MI_PAGE_FLAG_MASK);
  } while (!mi_atomic_cas_weak_release(&page->xthread_id, &xtid_old, xtid));

  #if MI_QSBR
  // keep the last owning thread id around for `mi_thread_id_of`
  if (tid > MI_THREAD_ID_DETACHED) { mi_page_qsbr_set_thread_id(page, tid); }
  #endif
}

static inline bool mi_page_is_abandoned(const mi_page_t* page) {
  // note: the xtheap field of an abandoned theap is set to the subproc (for fast reclaim-on-free)
  return (mi_page_thread_id(page) <= MI_THREAD_ID_ABANDONED_MAPPED);
}

static inline bool mi_page_is_abandoned_mapped(const mi_page_t* page) {
  return (mi_page_thread_id(page) == MI_THREAD_ID_ABANDONED_MAPPED);
}

static inline void mi_page_set_abandoned_mapped(mi_page_t* page) {
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_atomic_or_relaxed(&page->xthread_id, (mi_thread_id_t)MI_THREAD_ID_ABANDONED_MAPPED);
}

static inline void mi_page_clear_abandoned_mapped(mi_page_t* page) {
  mi_assert_internal(mi_page_is_abandoned_mapped(page));
  mi_atomic_and_relaxed(&page->xthread_id, (mi_thread_id_t)MI_PAGE_FLAG_MASK);
}


static inline mi_theap_t* mi_page_theap(const mi_page_t* page) {
  mi_assert_internal(!mi_page_is_abandoned(page));
  mi_assert_internal(page->theap != NULL && !_mi_is_empty_theap(page->theap));
  return page->theap;
}

static inline mi_tld_t* mi_page_tld(const mi_page_t* page) {
  mi_assert_internal(!mi_page_is_abandoned(page));
  mi_assert_internal(page->theap != NULL);
  return page->theap->tld;
}


static inline mi_heap_t* mi_page_heap(const mi_page_t* page) {
  mi_heap_t* heap = page->heap;
  mi_assert_internal(heap != NULL);
  return heap;
}

static inline mi_subproc_t* mi_page_subproc(const mi_page_t* page) {
  mi_heap_t* const heap = mi_page_heap(page);
  return heap->subproc;
}

//-----------------------------------------------------------
// Thread free list and ownership
//-----------------------------------------------------------

// Thread free flag helpers
static inline mi_block_t* mi_tf_block(mi_thread_free_t tf) {
  return (mi_block_t*)(tf & ~1);
}

static inline bool mi_tf_is_owned(mi_thread_free_t tf) {
  return ((tf & 1) == 1);
}

static inline mi_thread_free_t mi_tf_create(mi_block_t* block, bool owned) {
  const uintptr_t base = (uintptr_t)block | (owned ? 1 : 0);
  return (mi_thread_free_t)base;
}

static inline mi_thread_free_t mi_tf_set_owned(mi_thread_free_t tf, bool owned) {
  return mi_tf_create(mi_tf_block(tf), owned);
}

// Thread free access
static inline mi_block_t* mi_page_thread_free(const mi_page_t* page) {
  return mi_tf_block(mi_atomic_load_relaxed(&((mi_page_t*)page)->xthread_free));
}

// are there any available blocks?
static inline bool mi_page_has_any_available(const mi_page_t* page) {
  mi_assert_internal(page != NULL && page->reserved > 0);
  return (mi_page_used(page) < page->reserved || (mi_page_thread_free(page) != NULL));
}

// Owned?
static inline bool mi_page_is_owned(const mi_page_t* page) {
  return mi_tf_is_owned(mi_atomic_load_relaxed(&((mi_page_t*)page)->xthread_free));
}

// get ownership; returns true if the page was not owned before.
static inline bool mi_page_claim_ownership(mi_page_t* page) {
  const uintptr_t old = mi_atomic_or_acq_rel(&page->xthread_free, (uintptr_t)1);
  return ((old&1)==0);
}

/* -----------------------------------------------------------
  Delayed page release
----------------------------------------------------------- */

static inline void mi_page_qsbr_unlink(mi_page_t* page) {
#if MI_QSBR
  if (!mi_page_qsbr_is_in_queue(page)) {
    mi_assert_internal(page->qsbr_next == NULL && page->qsbr_prev == NULL);
    return;
  }
  mi_theap_t* theap = mi_page_theap(page);
  if (page->qsbr_prev != NULL) { page->qsbr_prev->qsbr_next = page->qsbr_next; }
  else {
    mi_assert_internal(theap->qsbr_delayed == page);
    theap->qsbr_delayed = page->qsbr_next;
  }
  if (page->qsbr_next != NULL) { page->qsbr_next->qsbr_prev = page->qsbr_prev; }
  else {
    mi_assert_internal(theap->qsbr_delayed_last == page);
    theap->qsbr_delayed_last = page->qsbr_prev;
  }
  page->qsbr_next = page->qsbr_prev = NULL;
  mi_page_qsbr_set_in_queue(page, false);
#else
  MI_UNUSED(page);
#endif
}

static inline void mi_page_qsbr_enqueue(mi_page_t* page, mi_theap_t* theap) {
#if MI_QSBR
  mi_assert_internal(mi_page_theap(page) == theap);
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(mi_heap_qsbr_is_enabled(mi_page_heap(page)));
  mi_assert_internal(!mi_page_qsbr_is_clear(&page->qsbr));
  if (mi_page_qsbr_is_in_queue(page)) return;
  mi_assert_internal(page->qsbr_next == NULL && page->qsbr_prev == NULL);
  page->qsbr_prev = theap->qsbr_delayed_last;
  if (theap->qsbr_delayed_last != NULL) { theap->qsbr_delayed_last->qsbr_next = page; }
  else { theap->qsbr_delayed = page; }
  theap->qsbr_delayed_last = page;
  mi_page_qsbr_set_in_queue(page, true);
#else
  MI_UNUSED(page);
  MI_UNUSED(theap);
#endif
}

static inline void _mi_page_qsbr_clear(mi_page_t* page) {
#if MI_QSBR
  mi_page_qsbr_unlink(page);
  mi_page_qsbr_clear(&page->qsbr);
#else
  MI_UNUSED(page);
#endif
}

static inline bool _mi_page_qsbr_is_clear(const mi_page_t* page) {
#if MI_QSBR
  return (!mi_page_qsbr_is_in_queue(page) && page->qsbr_next == NULL && page->qsbr_prev == NULL &&
          mi_page_qsbr_is_clear(&page->qsbr));
#else
  MI_UNUSED(page);
  return true;
#endif
}

static inline bool _mi_page_qsbr_should_delay(mi_page_t* page) {
#if MI_QSBR
  mi_assert_internal(mi_page_is_owned(page) && mi_page_all_free(page));
  mi_heap_t* heap = mi_page_heap(page);
  if (!mi_heap_qsbr_is_enabled(heap)) return false;
  const bool delayed = mi_page_qsbr_should_delay(&page->qsbr, &heap->qsbr, mi_page_capacity_size(page));
  mi_assert_internal(!delayed || !mi_page_qsbr_is_clear(&page->qsbr));
  return delayed;
#else
  MI_UNUSED(page);
  return false;
#endif
}

/* -------------------------------------------------------------------
Encoding/Decoding the free list next pointers

This is to protect against buffer overflow exploits where the
free list is mutated. Many hardened allocators xor the next pointer `p`
with a secret key `k`, as `p^k`. This is weak: if the attacker can guess
`p` this reveals `k`, and xor-ing two entries cancels the key. A non-linear
encoding like `((p^k2)<<<k1)+k1` avoids this but can still be broken with
just a few known `p`.

Instead, each page has two random keys (`keys[0]` and `keys[1]`), and each
link is bound to the address `b` of the block it is stored in (so links
cannot be copied). We use one of three encodings:

1. Tagged offsets (on 64-bit platforms).
   We store the 32-bit offset `v` of the next block relative to `b` (or relative to the page start with
   memory tagging or CHERI), with `NULL` as the reserved offset `MI_BLOCK_OFS_NULL`, together with a keyed 32-bit tag:

     x      = m ^ (v | (M(b,v) & H))
     M(b,v) = mulxor64(v ^ keys[0], b ^ keys[1])

   where `H` selects the upper 32 bits, `mulxor64` xor's the low and high halves of the full 128-bit
   product (or with `MI_SECURE=1` (`MI_ENCODE_FREELIST==1`) just the low 64 bits of the product, which is
   faster but makes it easier to recover the keys), and the mask `m = keys[0] <<< 32` hides the
   (randomized) free list order. (With memory tagging, the offset `v` is 24 bits and includes the
   memory tag of the next block, and `H` selects all other bits; see `MI_BLOCK_OFS_MASK`).
   Decoding is just an xor and an add; the tag
   is verified alongside with a predictable branch so the multiply mostly costs
   throughput. A corrupted or forged link validates with
   probability 2^-32, and it is difficult to recover the keys
   from known `(b,v,x)` triples (except for `m`), leaving ~96 bits of key.
   With `MI_PAC=WEAK` the process key `K` is also mixed in as
   `mulxor64(v ^ keys[0], b ^ keys[1] ^ K)` (see `prim/prim-pac.h:mi_keyed_mac` and `mi_keyed_mac_fast`).

2. Authenticated offsets (with `MI_PAC=ON`).
   As the tagged offsets, but the tag is computed with the (unreadable)
   hardware pointer authentication key:

     x      = keys[1] ^ (v | (M(b,v) & H))
     M(b,v) = let t = pacga(v, b ^ keys[0]) in t | (t >> 32)

   where `H` is as in the tagged offsets (the upper 32 bits, or with memory tagging all bits
   outside the 24-bit offset), and `pacga` returns a 32-bit code in the upper 32 bits (and zeros
   in the lower bits); the copy in the lower half provides the tag bits below bit 32 with memory tagging.
   The integrity of the links does not depend on the page keys: `keys[0]` serves as a nonce
   such that links from a previous use of the page do not validate, and a forged link validates
   with probability 2^-32 even if the page keys are known. The mask `keys[1]` still hides the
   (randomized) free list order (and the memory tags of the free blocks) from an attacker that
   can only read freed memory.

3. Xor-shift (the default on 32-bit, where we lack a cheap wide multiply).
   We store the full next address `p` (with `NULL` encoded as the page address):

     x    = L(p - t) <<< keys[0]
     t    = L(b ^ keys[0]) + keys[1]
     L(y) = y ^ (y << (MI_SIZE_BITS/2))      (which is its own inverse)

   The tweak `t` binds the link to the block `b`, the xor-shift `L` diffuses
   low bits into the high bits (such that partial overwrites decode outside
   the page), and the secret rotation prevents linear combinations of
   additions and xor's. Decoded links are validated by checking that they
   are in the page.

The padding canary encodes `NULL` at the unaligned address `b|1` (so it can
never be used as a valid free list link).
------------------------------------------------------------------- */

static inline bool mi_is_in_same_page(const void* p, const void* q) {
  mi_page_t* page = mi_ptr_page(p);
  return mi_page_contains_address(page,q);
  // return (mi_ptr_page(p) == mi_ptr_page(q));
}

// The inputs to encode the free list links of a page; these can be loaded once before a loop
// (as in a CAS loop where the compiler cannot hoist the loads from the page).
typedef struct mi_page_codec_s {
  const mi_page_t* page;
  #if (MI_ENCODE_FREELIST || MI_PADDING)
  uint8_t*         page_start;
  mi_addr_t        keys[2];
  #endif
} mi_page_codec_t;

static inline mi_page_codec_t mi_page_codec(const mi_page_t* page) {
  mi_page_codec_t codec;
  codec.page = page;
  #if (MI_ENCODE_FREELIST || MI_PADDING)
  codec.page_start = mi_page_start(page);
  codec.keys[0] = page->keys[0];
  codec.keys[1] = page->keys[1];
  #endif
  return codec;
}

#if (MI_ENCODE_FREELIST || MI_PADDING)
#if MI_SIZE_BITS>=64  // tagged offsets

// On 64-bit, a free list link stores the offset `d` of the next block (relative to the link address, or to the page start
// with memory tagging or CHERI), masked with
// a key, and a (keyed) MAC over `d` and the address of the link in all other bits (where the whole link is masked). With memory tagging, `d` includes
// the memory tag of the next block (at `MI_PTR_MEMTAG_MASK`, as the page start has tag 0) and we use a smaller
// offset to keep a longer MAC (pages with more than one block are at most `MI_LARGE_PAGE_SIZE`). Without memory
// tagging we use a 32-bit offset as that gives the best codegen (as the mask folds into the add, e.g. `uxtw` on arm64).
#define MI_BLOCK_OFS_BITS  (MI_PTR_MEMTAG_MASK==0 ? 32 : 24)
#define MI_BLOCK_OFS_MASK  (MI_PTR_MEMTAG_MASK | ((MI_ADDR(1) << MI_BLOCK_OFS_BITS) - 1))
#define MI_BLOCK_OFS_NULL  ((MI_ADDR(1) << MI_BLOCK_OFS_BITS) - 1)   // odd, so never a block offset

#if MI_LARGE_PAGE_SIZE >= (1UL<<24)
#error "the block offsets in a page with multiple blocks must fit in a free list link (see MI_BLOCK_OFS_BITS)"
#endif

#if MI_PAC>=2
static inline mi_addr_t mi_ptr_mac(const mi_page_codec_t* codec, mi_addr_t baddr, mi_addr_t d) {
  return mi_keyed_mac(d, baddr ^ codec->keys[0]);
}
static inline mi_addr_t mi_ptr_ofs_key(const mi_page_codec_t* codec) {
  return codec->keys[1];
}
#else
static inline mi_addr_t mi_ptr_mac(const mi_page_codec_t* codec, mi_addr_t baddr, mi_addr_t d) {
  #if MI_ENCODE_FREELIST==1
  return mi_keyed_mac_fast(d ^ codec->keys[0], baddr ^ codec->keys[1]);
  #else
  return mi_keyed_mac(d ^ codec->keys[0], baddr ^ codec->keys[1]);
  #endif
}
static inline mi_addr_t mi_ptr_ofs_key(const mi_page_codec_t* codec) {
  return mi_rotl(codec->keys[0], 32);
}
#endif

// The offset is relative to the link address `baddr` (as a signed 32-bit value) such that decoding does not
// need the page start (but with memory tagging or CHERI we need the page start for the tag or capability).
#if !MI_MTE && !MI_CHERI
#define MI_BLOCK_OFS_RELATIVE  1
#else
#define MI_BLOCK_OFS_RELATIVE  0
#endif

// Encode a pointer `p` (in the page, or NULL) to be stored at address `baddr`
static inline mi_encoded_t mi_ptr_encode(const mi_page_codec_t* codec, mi_addr_t baddr, const void* p) {
  mi_assert_internal(p==NULL || mi_page_contains_address(codec->page,p));
  #if MI_BLOCK_OFS_RELATIVE
  const mi_addr_t d = (p==NULL ? MI_BLOCK_OFS_NULL : (mi_addr(p) - baddr) & MI_BLOCK_OFS_MASK);
  #else
  const mi_addr_t d = (p==NULL ? MI_BLOCK_OFS_NULL : mi_ptr_pack(p) - mi_addr(codec->page_start));
  #endif
  return (mi_encoded_t)(mi_ptr_ofs_key(codec) ^ (d | (mi_ptr_mac(codec,baddr,d) & ~MI_BLOCK_OFS_MASK)));
}

// Decode a value `x` stored at address `baddr`; `valid` is false if `x` was not a valid encoding
static inline void* mi_ptr_decode(const mi_page_codec_t* codec, mi_addr_t baddr, mi_encoded_t x, bool* valid) {
  const mi_addr_t y = (mi_addr_t)x ^ mi_ptr_ofs_key(codec);
  const mi_addr_t d = y & MI_BLOCK_OFS_MASK;
  *valid = (((y ^ mi_ptr_mac(codec,baddr,d)) & ~MI_BLOCK_OFS_MASK) == 0);
  #if MI_BLOCK_OFS_RELATIVE
  return (d == MI_BLOCK_OFS_NULL ? NULL : (void*)(baddr + (mi_addr_t)(intptr_t)(int32_t)(uint32_t)d));
  #else
  return (d == MI_BLOCK_OFS_NULL ? NULL : codec->page_start + d);
  #endif
}

#else

static inline mi_addr_t mi_xorshift_half(mi_addr_t y) {
  return (y ^ (y << (MI_SIZE_BITS/2)));
}

static inline mi_addr_t mi_ptr_tweak(const mi_page_codec_t* codec, mi_addr_t baddr) {
  return (mi_xorshift_half(baddr ^ codec->keys[0]) + codec->keys[1]);
}

// Encode a pointer `p` (in the page, or NULL) to be stored at address `baddr`
static inline mi_encoded_t mi_ptr_encode(const mi_page_codec_t* codec, mi_addr_t baddr, const void* p) {
  mi_assert_internal(p==NULL || mi_page_contains_address(codec->page,p));
  const mi_addr_tagged_t addrt = (p==NULL ? (mi_addr_tagged_t)mi_addr(codec->page) : mi_ptr_pack(p));
  return (mi_encoded_t)mi_rotl(mi_xorshift_half(addrt - mi_ptr_tweak(codec,baddr)), codec->keys[0]);
}

// Decode a value `x` stored at address `baddr`; `valid` is false if `x` was not a valid encoding
static inline void* mi_ptr_decode(const mi_page_codec_t* codec, mi_addr_t baddr, mi_encoded_t x, bool* valid) {
  const mi_addr_tagged_t addrt = mi_xorshift_half(mi_rotr((mi_addr_t)x, codec->keys[0])) + mi_ptr_tweak(codec,baddr);
  if (addrt == (mi_addr_tagged_t)mi_addr(codec->page)) { *valid = true; return NULL; }
  void* const p = mi_ptr_unpack(codec->page_start, addrt);
  *valid = mi_page_contains_address(codec->page, p);
  return p;
}

#endif

static inline uint32_t mi_ptr_encode_canary(const mi_page_t* page, const mi_block_t* block) {
  const mi_page_codec_t codec = mi_page_codec(page);
  const mi_addr_t e = (mi_addr_t)mi_ptr_encode(&codec, mi_addr(block)|1, NULL);
  const uint32_t x = (uint32_t)(e ^ (e >> (MI_SIZE_BITS/2)));
  // make the lowest byte 0 to prevent spurious read overflows which could be a security issue (issue #951)
  // also clear bit 9 which we set only when a block is freed.
  #if MI_BIG_ENDIAN
  return (x & 0x00FFFEFF);
  #else
  return (x & 0xFFFFFE00);
  #endif
}

static inline uint32_t mi_ptr_encode_canary_freed(void) {
  return (0x00DEAD00);  // set bit 9 so it is different from any valid canary
}

static inline bool mi_ptr_decode_canary_is_freed(uint32_t canary) {
  return (canary == mi_ptr_encode_canary_freed());
}

#endif // MI_ENCODE_FREELIST || MI_PADDING

// Is `next` an aligned link inside the committed blocks of the page? (used to detect use-after-free writes in free lists
// that are not encoded, as encoded links are already validated by `mi_block_decode`)
static inline bool mi_block_next_is_valid(const mi_page_t* page, const mi_block_t* next, size_t capacity_size) {
  const mi_addr_t ofs = mi_addr(next) - mi_addr(mi_page_start(page));
  return (ofs < capacity_size && ofs % MI_INTPTR_SIZE == 0);
}

// Decode the free list link `x` stored in `block`; `valid` is false if the link was corrupted
static inline mi_block_t* mi_block_decode(const mi_page_t* page, const mi_block_t* block, mi_encoded_t x, bool* valid) {
  #if MI_ENCODE_FREELIST
  const mi_page_codec_t codec = mi_page_codec(page);
  return (mi_block_t*)mi_ptr_decode(&codec, mi_addr(block), x, valid);
  #else
  MI_UNUSED(block);
  *valid = true;
  const mi_addr_tagged_t addrt = (mi_addr_tagged_t)x;
  return (addrt==0 ? NULL : (mi_block_t*)mi_ptr_unpack(mi_page_start(page), addrt));
  #endif
}

static inline mi_encoded_t mi_block_encode(const mi_page_codec_t* codec, const mi_block_t* block, const mi_block_t* next) {
  #if MI_ENCODE_FREELIST
  return mi_ptr_encode(codec, mi_addr(block), next);
  #else
  MI_UNUSED(codec); MI_UNUSED(block);
  return (mi_encoded_t)mi_ptr_pack(next);  // never store a capability in a (freed) block
  #endif
}

static inline mi_encoded_t mi_block_link(const mi_block_t* block) {
  mi_track_mem_defined(block,sizeof(mi_block_t));
  const mi_encoded_t x = mi_atomic_load_relaxed(&block->next);
  mi_track_mem_noaccess(block,sizeof(mi_block_t));
  return x;
}

static inline mi_block_t* mi_block_next(const mi_page_t* page, const mi_block_t* block) {
  const mi_encoded_t x = mi_block_link(block);
  bool valid;
  mi_block_t* const next = mi_block_decode(page, block, x, &valid);
  if mi_unlikely(!valid) {
    return _mi_error_block_next_is_corrupted(page, block, x); // returns NULL
  }
  return next;
}

static inline void mi_block_set_next_ex(const mi_page_codec_t* codec, mi_block_t* block, const mi_block_t* next) {
  mi_track_mem_undefined(block,sizeof(mi_block_t));
  mi_atomic_store_relaxed(&block->next, mi_block_encode(codec, block, next));
  mi_track_mem_noaccess(block,sizeof(mi_block_t));
}

static inline void mi_block_set_next(const mi_page_t* page, mi_block_t* block, const mi_block_t* next) {
  const mi_page_codec_t codec = mi_page_codec(page);
  mi_block_set_next_ex(&codec, block, next);
}


/* -----------------------------------------------------------
  Safe unlinking of doubly linked lists

  In secure mode, check that a `node` is properly linked through its `prev` and `next`
  fields before unlinking it, such that corrupted links cannot be used for arbitrary writes.
  `first` is the head of the list, and `last` the tail (or `NULL` if the list has no tail).
  A corrupted list is a fatal error.
----------------------------------------------------------- */

static inline bool mi_list_link_is(const void* link, const void* node) {
  return (mi_addr(link) == mi_addr(node));  // ignore memory tags
}

#if MI_SECURE>=1
#define mi_list_unlink_check(node,prev,next,first,last) \
  do { \
    if mi_unlikely(!((node)->prev==NULL ? mi_list_link_is(first,node) : mi_list_link_is((node)->prev->next,node)) || \
                   !((node)->next==NULL ? ((last)==NULL || mi_list_link_is(last,node)) : mi_list_link_is((node)->next->prev,node))) \
    { _mi_error_list_corrupted(node); } \
  } while(0)
#else
#define mi_list_unlink_check(node,prev,next,first,last)  do { } while(0)
#endif

#endif
