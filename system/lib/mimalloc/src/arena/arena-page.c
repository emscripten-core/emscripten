/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/prim/prim-tls.h"
#include "mimalloc/util/stats.h"
#include "mimalloc/util/memid.h"
#include "mimalloc/alloc/page.h"
#include "mimalloc/arena/arena-alloc.h"
#include "mimalloc/profile/sample-guarded.h"  // _mi_page_unguard_all
#include "mimalloc/arena/arena-page.h"
#include "arena-page.h"

/* -----------------------------------------------------------
  Util
----------------------------------------------------------- */

#if MI_DEBUG > 1
static bool mi_heap_has_page(mi_heap_t* heap, mi_arena_t* arena, mi_page_t* page) {
  mi_assert(arena->arena_idx < MI_MAX_ARENAS);
  mi_arena_pages_t* arena_pages = mi_atomic_load_ptr_relaxed(mi_arena_pages_t, &heap->arena_pages[arena->arena_idx]);
  return (page->memid.memkind == MI_MEM_ARENA &&
          mi_memid_arena(page->memid) == arena &&
          arena_pages != NULL &&
          mi_bitmap_is_setN(arena_pages->pages, page->memid.mem.arena.slice_index, 1));
}
#endif


mi_page_t* mi_arena_page_at_slice(mi_arena_t* arena, size_t slice_index) {
  mi_assert_internal(slice_index < arena->slice_count);
  #if MI_PAGE_META_IS_ALIGNED
  mi_page_t* const page = mi_aligned_ptr_page(mi_arena_slice_start(arena,slice_index)); // todo: optimize?
  return page;
  #else
  if (arena->pages_meta != NULL) {
    mi_page_t* const page = mi_page_meta_load(mi_arena_page_meta_at_slice(arena, slice_index));
    #if MI_PAGE_META_SMALL_IS_ALIGNED
    // pages with small blocks still have the page at the start of the slice (and set the `block_size` in pages_meta to 0)
    if (page->block_size>0) return mi_page_meta_bounded(page);
    #else
    return mi_page_meta_bounded(page);
    #endif
  }
  // fall through (for MI_PAGE_META_SMALL_IS_ALIGNED)
  return mi_page_meta_bounded(mi_page_meta_load((mi_page_t*)mi_arena_slice_start(arena,slice_index)));
  #endif
}

/* -----------------------------------------------------------
  Arena pages: these bitmaps track abandoned pages per size
  bin, and the overall pages belonging to a particular heap.
----------------------------------------------------------- */


static size_t mi_arena_pages_size(size_t slice_count, size_t* bitmap_base) {
  if (slice_count == 0) slice_count = MI_BCHUNK_BITS;
  mi_assert_internal((slice_count % MI_BCHUNK_BITS) == 0);
  const size_t base_size = mi_align_up(sizeof(mi_arena_pages_t), MI_BCHUNK_SIZE);
  const size_t bitmaps_count = 1 + MI_ARENA_BIN_COUNT; // pages, and abandoned
  const size_t bitmaps_size = bitmaps_count * mi_bitmap_size(slice_count, NULL);
  const size_t size = base_size + bitmaps_size;
  if (bitmap_base != NULL) *bitmap_base = base_size;
  return size;
}

// allocate initial arena_pages from the main heap (or as meta data in secure mode)
static mi_arena_pages_t* mi_arena_pages_alloc(mi_arena_t* arena) {
  const size_t slice_count = arena->slice_count;
  size_t bitmap_base = 0;
  const size_t size = mi_arena_pages_size(slice_count, &bitmap_base);
  #if MI_SECURE
  mi_arena_pages_t* arena_pages = (mi_arena_pages_t*)_mi_meta_zalloc_aligned(arena->subproc, size, MI_BCHUNK_SIZE, NULL);
  #else
  mi_arena_pages_t* arena_pages = (mi_arena_pages_t*)mi_heap_zalloc_aligned(arena->subproc->heap_main, size, MI_BCHUNK_SIZE);
  #endif
  if (arena_pages==NULL) return NULL;
  uint8_t* base = (uint8_t*)arena_pages + bitmap_base;
  mi_assert_internal(mi_is_aligned(base, MI_BCHUNK_SIZE));
  arena_pages->pages = mi_arena_bitmap_init(slice_count, &base);
  for (size_t i = 0; i < MI_ARENA_BIN_COUNT; i++) {
    arena_pages->pages_abandoned[i] = mi_arena_bitmap_init_lazy(slice_count, &base);  // initialized on first abandonment
  }
  return arena_pages;
}

mi_arena_pages_t* _mi_heap_ensure_arena_pages(mi_heap_t* heap, mi_arena_t* arena) {
  mi_assert_internal(arena!=NULL);
  mi_assert_internal(heap!=NULL);
  mi_assert(arena->arena_idx < MI_MAX_ARENAS);
  mi_arena_pages_t* arena_pages = mi_heap_arena_pages(heap, arena);
  if (arena_pages==NULL) {
    mi_lock(&heap->arena_pages_lock) {
      arena_pages = mi_atomic_load_ptr_acquire(mi_arena_pages_t, &heap->arena_pages[arena->arena_idx]);
      if (arena_pages == NULL) {  // still NULL?
        if (mi_is_heap_main(heap)) {
          // the page info for the main heap is always allocated as part of an arena
          arena_pages = &arena->pages_main;
        }
        else {
          // always allocate the arena pages info from the main heap
          // todo: allocate into the current arena?
          arena_pages = mi_arena_pages_alloc(arena);
        }
        mi_atomic_store_ptr_release(mi_arena_pages_t, &heap->arena_pages[arena->arena_idx], arena_pages);
      }
    }
  }
  if (mi_is_heap_main(heap)) { mi_assert(arena_pages != NULL); }  // can never fail
  return arena_pages;
}


mi_arena_t* _mi_page_arena_pages(mi_page_t* page, size_t* slice_index, size_t* slice_count, mi_arena_pages_t** parena_pages) {
  // todo: maybe store the arena* directly in the page?
  mi_assert_internal(mi_page_is_owned(page));
  mi_arena_t* const arena = mi_arena_from_memid(page->memid, slice_index, slice_count);
  mi_assert_internal(arena != NULL);
  if (parena_pages != NULL) {
    mi_heap_t* heap = mi_page_heap(page);
    mi_arena_pages_t* const arena_pages = mi_heap_arena_pages(heap, arena);
    mi_assert_internal(arena_pages != NULL);
    // note: we cannot assert the page bit is set as moving heap pages may temporarily clear it (see `mi_heap_move_page_at`)
    *parena_pages = arena_pages;
  }
  return arena;
}

static bool mi_page_has_guards(const mi_page_t* page, mi_page_guards_t guards) {
  return (page->memid.guard_front == (guards.front > 0) && page->memid.guard_end == (guards.end > 0));
}


/* -----------------------------------------------------------
  Claim abandoned pages
----------------------------------------------------------- */

typedef struct mi_arena_claim_info_s {
  mi_arena_t*       arena;
  mi_page_guards_t  guards;
} mi_arena_claim_info_t;

static bool mi_arena_try_claim_abandoned(size_t slice_index, void* arg, bool* keep_abandoned) {
  const mi_arena_claim_info_t* const info = (const mi_arena_claim_info_t*)arg;
  // found an abandoned page of the right size
  mi_page_t* const page  = mi_arena_page_at_slice(info->arena, slice_index);
  // only reclaim pages with the same guard pages as a fresh page (so meta data and user pages are never mixed)
  if (!mi_page_has_guards(page, info->guards)) {
    *keep_abandoned = true;
    return false;
  }
  // can we claim ownership?
  if (!mi_page_claim_ownership(page)) {
    // there was a concurrent free that reclaims this page ..
    // we need to keep it in the abandoned map as the free will call `mi_arena_page_unabandon`,
    // and wait for readers (us!) to finish. This is why it is very important to set the abandoned
    // bit again (or otherwise the unabandon will never stop waiting).
    *keep_abandoned = true;
    return false;
  }
  else {
    // yes, we can reclaim it, keep the abandoned map entry clear
    *keep_abandoned = false;
    return true;
  }
}


typedef struct mi_arena_abandoned_visit_info_s {
  mi_theap_t* theap;
  size_t      slice_count;
  size_t      block_size;
  size_t      bin;
  mi_page_guards_t guards;
} mi_arena_abandoned_visit_info_t;

static bool mi_arena_try_find_abandoned_visit(mi_arena_t* arena, const void* arg, void** result) {
  const mi_arena_abandoned_visit_info_t* info = (const mi_arena_abandoned_visit_info_t*)arg;
  mi_assert_internal(result != NULL);
  mi_theap_t* const theap = info->theap;
  mi_heap_t* const heap = mi_theap_heap(theap);
  const size_t tseq = theap->tld->thread_seq;
  mi_arena_pages_t* const arena_pages = mi_heap_arena_pages(heap, arena);
  if (arena_pages != NULL) {
    size_t slice_index;
    mi_bitmap_t* const bitmap = arena_pages->pages_abandoned[info->bin];
    mi_arena_claim_info_t claim_info = { arena, info->guards };
    if (mi_bitmap_try_find_and_claim(bitmap, tseq, &slice_index, &mi_arena_try_claim_abandoned, &claim_info)) {
      // found an abandoned page of the right size
      // and claimed ownership.
      mi_page_t* page = mi_arena_page_at_slice(arena, slice_index);
      mi_assert_internal(mi_page_is_owned(page));
      mi_assert_internal(mi_page_is_abandoned(page));
      mi_assert_internal(mi_heap_has_page(heap, arena, page));
      mi_page_clear_abandoned_mapped(page);  // no longer in the abandoned map
      mi_atomic_decrement_relaxed(&heap->abandoned_count[info->bin]);
      mi_theap_stat_decrease(theap, pages_abandoned, 1);
      mi_theap_stat_counter_increase(theap, pages_reclaim_on_alloc, 1);

      _mi_page_free_collect(page, false);  // update `used` count
      mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, info->slice_count));
      mi_assert_internal(mi_page_slice_committed(page) > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, info->slice_count));
      mi_assert_internal(mi_bitmap_is_setN(arena->slices_dirty, slice_index, info->slice_count));
      mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
      mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
      mi_assert_internal(mi_page_block_size(page) == info->block_size);
      mi_assert_internal(!mi_page_is_full(page));
      *result = page;
      return false; // break
    }
  }
  return true; // continue
}

mi_page_t* _mi_arenas_page_try_find_abandoned(mi_theap_t* theap, size_t slice_count, size_t block_size, mi_page_guards_t guards)
{
  mi_heap_t* const heap = mi_theap_heap(theap);
  const size_t tseq = theap->tld->thread_seq;
  mi_arena_t* const req_arena = heap->exclusive_arena;

  const size_t bin = _mi_bin(block_size);
  if (bin >= MI_ARENA_BIN_COUNT) {
    return NULL; // singleton page size
  }

  // any abandoned in our size class?
  mi_assert_internal(heap != NULL);
  if (mi_atomic_load_relaxed(&heap->abandoned_count[bin]) == 0) {
    return NULL;
  }

  // search arena's
  const bool allow_large = true;
  const int  any_numa = -1;
  const bool match_numa = true;
  const mi_arena_abandoned_visit_info_t info = { theap, slice_count, block_size, bin, guards };
  void* page = NULL;
  mi_forall_suitable_arenas(heap, req_arena, tseq, match_numa, any_numa, allow_large, &mi_arena_try_find_abandoned_visit, &info, &page);
  return (mi_page_t*)page;
}


/* -----------------------------------------------------------
  Abandon a page
----------------------------------------------------------- */


// release ownership of a page. This may free the page if all blocks were concurrently
// freed in the meantime. Returns true if the page was freed.
bool _mi_abandoned_page_unown(mi_page_t* page, mi_theap_t* current_theapx) {
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(mi_theap_matches_thread(current_theapx));
  if (mi_page_is_singleton(page) && mi_page_all_free(page)) {
    // an empty singleton is never found again by allocation or frees, so release or delay it now
    // (this does not recurse as `_mi_arenas_page_free` never abandons a singleton)
    _mi_arenas_page_unabandon(page, current_theapx);
    return _mi_arenas_page_free(page, current_theapx);
  }
  mi_thread_free_t tf_new;
  mi_thread_free_t tf_old = mi_atomic_load_relaxed(&page->xthread_free);
  do {
    mi_assert_internal(mi_tf_is_owned(tf_old));
    while mi_unlikely(mi_tf_block(tf_old) != NULL) {
      _mi_page_free_collect(page, false);  // update used
      if (mi_page_all_free(page)) {
        _mi_arenas_page_unabandon(page, current_theapx);
        return _mi_arenas_page_free(page, current_theapx);
      }
      tf_old = mi_atomic_load_relaxed(&page->xthread_free);
    }
    mi_assert_internal(mi_tf_block(tf_old)==NULL);
    tf_new = mi_tf_create(NULL, false);
  } while (!mi_atomic_cas_weak_release(&page->xthread_free, &tf_old, tf_new));
  return false;
}


void _mi_arenas_page_abandon(mi_page_t* page, mi_theap_t* current_theapx) {
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(MI_QSBR || !mi_page_all_free(page));
  mi_assert_internal(page->next==NULL && page->prev == NULL);
  mi_assert_internal(mi_theap_matches_thread(current_theapx));
  // mi_assert_internal(current_theap == _mi_page_associated_theap(page));
  #if MI_QSBR
  mi_assert_internal(!mi_page_qsbr_is_in_queue(page) && page->qsbr_next == NULL && page->qsbr_prev == NULL);
  #endif

  // note: somewhat expensive to update here, but might be good as then we attribute
  // the current allocations/frees to the current thread/theap. Otherwise it might be
  // reclaimed later in another thread/theap and those allocations/frees get attributed there...
  _mi_page_update_stats(page);

  // add to abandoned?
  mi_heap_t* heap = mi_page_heap(page);
  if (page->memid.memkind==MI_MEM_ARENA && !mi_page_is_full(page)
      #if MI_QSBR
      && !mi_page_is_singleton(page) // an arena singleton has size > MI_LARGE_MAX_OBJ_SIZE and bin==MI_BIN_HUGE
      #endif
     ) {
    // make available for allocations
    size_t bin = _mi_bin(mi_page_block_size(page));
    mi_assert_internal(bin < MI_ARENA_BIN_COUNT);
    if (bin < MI_ARENA_BIN_COUNT) { // paranoia
      size_t slice_index;
      size_t slice_count;
      mi_arena_pages_t* arena_pages = NULL;
      mi_arena_t* const arena = _mi_page_arena_pages(page, &slice_index, &slice_count, &arena_pages); MI_UNUSED(arena);

      mi_assert_internal(!mi_page_is_singleton(page));
      mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
      mi_assert_internal(mi_page_slice_committed(page) > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
      mi_assert_internal(mi_bitmap_is_setN(arena->slices_dirty, slice_index, slice_count));

      mi_page_set_abandoned_mapped(page);
      mi_bitmap_ensure_init(arena_pages->pages_abandoned[bin], arena->slice_count);
      const bool was_clear = mi_bitmap_set(arena_pages->pages_abandoned[bin], slice_index);
      MI_UNUSED(was_clear); mi_assert_internal(was_clear);
      mi_atomic_increment_relaxed(&heap->abandoned_count[bin]);
      mi_theapx_stat_increase(heap, current_theapx, pages_abandoned, 1);
      _mi_abandoned_page_unown(page, current_theapx);
      return;
    }
  }
  // otherwise,
  // page is full (or a singleton), or the page is OS/externally allocated
  // leave as is; it will be reclaimed when an object is free'd in the page
  // but for non-arena pages, add to the subproc list so these can be visited
  if (page->memid.memkind != MI_MEM_ARENA) {
    mi_lock(&heap->os_abandoned_pages_lock) {
      // push in front
      page->prev = NULL;
      page->next = heap->os_abandoned_pages;
      if (page->next != NULL) { page->next->prev = page; }
      heap->os_abandoned_pages = page;
    }
    mi_theapx_stat_increase(heap, current_theapx, pages_os_abandoned, 1);
  }
  mi_theapx_stat_increase(heap, current_theapx, pages_abandoned, 1);
  _mi_abandoned_page_unown(page, current_theapx);
}


// this is called from `alloc/free.c:mi_free_try_collect_mt` only.
bool _mi_arenas_page_try_reabandon_to_mapped(mi_page_t* page) {
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(!mi_page_is_abandoned_mapped(page));
  mi_assert_internal(!mi_page_is_full(page));
  mi_assert_internal(!mi_page_all_free(page));
  mi_assert_internal(!mi_page_is_singleton(page));
  if (mi_page_is_full(page) || mi_page_is_abandoned_mapped(page) || page->memid.memkind != MI_MEM_ARENA) {
    return false;
  }
  else {
    // do not use _mi_heap_theap as we may call this during shutdown of threads and don't want to reinitialize the theap
    mi_theap_t* const theapx = _mi_page_associated_theap_peek(page); // can be NULL
    mi_heap_t* const heap = mi_page_heap(page);
    // if (theapx==NULL) return false;
    mi_theapx_stat_counter_increase(heap, theapx, pages_reabandon_full, 1);
    mi_theapx_stat_adjust_decrease(heap, theapx, pages_abandoned, 1);  // adjust as we are not abandoning fresh
    _mi_arenas_page_abandon(page, theapx);
    return true;
  }
}

// called from `mi_free` if trying to unabandon an abandoned page
void _mi_arenas_page_unabandon(mi_page_t* page, mi_theap_t* current_theapx) {
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(mi_theap_matches_thread(current_theapx));

  mi_heap_t* const heap = mi_page_heap(page);
  if (mi_page_is_abandoned_mapped(page)) {
    mi_assert_internal(page->memid.memkind==MI_MEM_ARENA);
    // remove from the abandoned map
    const size_t bin = _mi_bin(mi_page_block_size(page));
    mi_assert_internal(bin < MI_ARENA_BIN_COUNT);
    size_t slice_index;
    size_t slice_count;
    mi_arena_pages_t* arena_pages;
    mi_arena_t* arena = _mi_page_arena_pages(page, &slice_index, &slice_count, &arena_pages);  MI_UNUSED(arena);

    mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
    mi_assert_internal(mi_page_slice_committed(page) > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));

    // this busy waits until a concurrent reader (from alloc_abandoned) is done
    mi_bitmap_clear_once_set(arena->subproc, arena_pages->pages_abandoned[bin], slice_index);
    mi_page_clear_abandoned_mapped(page);
    mi_atomic_decrement_relaxed(&heap->abandoned_count[bin]);
  }
  else {
    // page is full (or a singleton), page is OS allocated
    // if not an arena page, remove from the subproc os pages list
    if (page->memid.memkind != MI_MEM_ARENA) {
      mi_lock(&heap->os_abandoned_pages_lock) {
        mi_list_unlink_check(page, prev, next, heap->os_abandoned_pages, NULL);
        if (page->prev != NULL) { page->prev->next = page->next; }
        if (page->next != NULL) { page->next->prev = page->prev; }
        if (heap->os_abandoned_pages == page) { heap->os_abandoned_pages = page->next; }
        page->next = NULL;
        page->prev = NULL;
      }
      mi_theapx_stat_decrease(heap, current_theapx, pages_os_abandoned, 1);
    }
  }
  mi_theapx_stat_decrease(heap, current_theapx, pages_abandoned, 1);
}
