/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
Visit, collect, and move or destroy all the pages of a heap.
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

static void mi_heap_qsbr_collect_delayed(mi_heap_t* heap, size_t max_pages, bool destroy);

/* -----------------------------------------------------------
  Visit all pages and blocks in a heap
----------------------------------------------------------- */

typedef struct mi_heap_visit_info_s {
  mi_heap_t*          heap;
  mi_block_visit_fun* visitor;
  void*               arg;
  bool                visit_blocks;
  bool                abandoned_only;
} mi_heap_visit_info_t;

// Collect a claimed abandoned page for a visit and unown it again. Returns `true` if the page was freed.
// Unlike `_mi_abandoned_page_unown`, the collect is forced so the `local_free` list is appended as well
// (`_mi_theap_area_visit_blocks` only treats blocks in the `free` list as free).
static bool mi_heap_visit_page_collect_claimed(mi_page_t* page) {
  if (_mi_page_free_collect(page, true) && mi_page_all_free(page)) {
    _mi_arenas_page_unabandon(page, NULL);
    return _mi_arenas_page_free(page, NULL);
  }
  return _mi_abandoned_page_unown(page, NULL);  // still frees an empty singleton (and any later cross-thread frees)
}

static bool mi_heap_visit_page(mi_page_t* page, const mi_heap_visit_info_t* vinfo) {
  mi_heap_area_t area;
  // abandoned pages can only be collected if we can claim them
  const bool abandoned = mi_page_is_abandoned(page);
  if (!abandoned) { _mi_page_free_collect(page, true); }  // collect cross-thread free'd blocks so `area.used` is precise.
  else if (mi_page_claim_ownership(page) && mi_heap_visit_page_collect_claimed(page)) return true;  // the page was freed
  _mi_heap_area_init(&area, page);
  mi_assert_internal(vinfo->heap == mi_page_heap(page));
  if (!vinfo->visitor(vinfo->heap, &area, NULL, area.block_size, vinfo->arg)) {
    return false; // break
  }
  if (vinfo->visit_blocks) {
    return _mi_theap_area_visit_blocks(&area, page, vinfo->visitor, vinfo->arg); // continue or break
  }
  else {
    return true; // continue
  }
}

static bool mi_heap_visit_page_at(size_t slice_index, size_t slice_count, mi_arena_t* arena, void* arg) {
  MI_UNUSED(slice_count);
  const mi_heap_visit_info_t* vinfo = (const mi_heap_visit_info_t*)arg;
  mi_page_t* page = mi_arena_page_at_slice(arena, slice_index);
  return mi_heap_visit_page(page, vinfo); // continue or break
}

static bool mi_heap_visit_arena(mi_arena_t* arena, const void* arg, void** result) {
  MI_UNUSED(result);
  const mi_heap_visit_info_t* vinfo = (const mi_heap_visit_info_t*)arg;
  mi_heap_visit_info_t visit_info = *vinfo;
  mi_heap_t* const heap = vinfo->heap;
  mi_arena_pages_t* arena_pages = mi_heap_arena_pages(heap, arena);
  if (arena_pages == NULL) return true; // continue
  if (vinfo->abandoned_only) {
    for (size_t bin = 0; bin < MI_ARENA_BIN_COUNT; bin++) {
      // TODO: if we had a single abandoned page map as well, this can be faster. Maybe for CPython?
      if (mi_atomic_load_relaxed(&heap->abandoned_count[bin]) > 0) {
        if (!_mi_bitmap_forall_set(arena_pages->pages_abandoned[bin], &mi_heap_visit_page_at, arena, &visit_info)) return false; // break
      }
    }
    return true; // continue
  }
  else {
    return _mi_bitmap_forall_set(arena_pages->pages, &mi_heap_visit_page_at, arena, &visit_info); // continue or break
  }
}

// The caller holds heap->theaps_lock to prevent thread-exit abandonment and theap removal.
static bool mi_heap_visit_owned_os_blocks(mi_heap_t* heap, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  const mi_heap_visit_info_t visit_info = { heap, visitor, arg, visit_blocks, false };
  for (mi_theap_t* theap = heap->theaps; theap != NULL; theap = theap->hnext) {
    if (theap->page_count_os == 0) continue;
    for (size_t bin = 0; bin <= MI_BIN_FULL; bin++) {
      mi_page_t* page = theap->pages[bin].first;
      while (page != NULL) {
        mi_page_t* const next = page->next;
        if (page->memid.memkind != MI_MEM_ARENA && !mi_heap_visit_page(page, &visit_info)) return false;
        page = next;
      }
    }
  }
  return true;
}

static bool mi_heap_visit_blocks_locked(mi_heap_t* heap, bool abandoned_only, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  // visit all pages in a heap
  // Other threads must not allocate in or free into this heap; thread-exit abandonment is blocked by the heap lock.
  const mi_heap_visit_info_t visit_info = { heap, visitor, arg, visit_blocks, abandoned_only };
  bool ok = mi_forall_arenas(heap, NULL, 0, &mi_heap_visit_arena, &visit_info, NULL);
  if (!ok) return false;

  mi_page_t* page = NULL;
  #if MI_QSBR
  if (!abandoned_only) {
    // Visit before abandoned OS pages, as collecting their frees may move them to the heap delayed list.
    // Arena-backed delayed pages were already visited through the all-pages map.
    page = mi_atomic_load_ptr_relaxed(mi_page_t, &heap->qsbr_delayed);
    while (ok && page != NULL) {
      mi_page_t* next = page->next;
      if (page->memid.memkind != MI_MEM_ARENA) { ok = mi_heap_visit_page(page, &visit_info); }
      page = next;
    }
  }
  #endif

  // visit abandoned pages in OS allocated memory
  mi_lock(&heap->os_abandoned_pages_lock) {
    page = heap->os_abandoned_pages;
  }
  while (ok && page != NULL) {
    mi_page_t* next = page->next;  // read upfront in case the visitor frees the page
    ok = mi_heap_visit_page(page, &visit_info);
    page = next;
  }

  // visit owned OS allocated blocks in the theaps
  if (ok && !abandoned_only) {
    ok = mi_heap_visit_owned_os_blocks(heap, visit_blocks, visitor, arg);
  }
  return ok;
}

bool _mi_heap_visit_blocks(mi_heap_t* heap, bool abandoned_only, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  mi_assert(visitor!=NULL);
  if (visitor==NULL) return false;
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return false;
  bool ok = false;
  mi_lock(&heap->theaps_lock) {
    ok = mi_heap_visit_blocks_locked(heap, abandoned_only, visit_blocks, visitor, arg);
  }
  return ok;
}

bool mi_heap_visit_blocks(mi_heap_t* heap, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  return _mi_heap_visit_blocks(heap, false, visit_blocks, visitor, arg);
}

bool mi_heap_visit_abandoned_blocks(mi_heap_t* heap, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  return _mi_heap_visit_blocks(heap, true, visit_blocks, visitor, arg);
}

/* -----------------------------------------------------------
  Move all pages of a deleted heap to a target heap: empty pages are freed,
  and if there is no target heap (`heap_target==NULL`) all pages are destroyed.
  At this point all theaps are freed and all pages are abandoned. A concurrent free can
  still own a page briefly; we skip such pages and retry them in a next round.
  When no page was skipped the heap has no pages left, as no new pages can be added.
  A free unlinks an abandoned OS page before freeing it, so we also retry until `os_page_count` is zero.
  This terminates since a free only owns a page for a bounded time (and then frees it,
  re-abandons it, or unowns it).
----------------------------------------------------------- */

typedef struct mi_heap_move_info_s {
  mi_heap_t*  heap;
  mi_heap_t*  heap_target;   // NULL to destroy the pages
  mi_theap_t* theap_target;
  bool        retry;         // set if a page could not be claimed (as it is owned by a concurrent free)
} mi_heap_move_info_t;

// Free, destroy, or move an owned abandoned page to the target heap
static void mi_heap_move_page(const mi_heap_move_info_t* info, mi_page_t* page) {
  mi_heap_t* const heap  = info->heap;
  mi_heap_t* heap_target = info->heap_target;
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(mi_page_heap(page) == heap);
  mi_assert_internal(heap_target == NULL || !mi_heap_qsbr_is_enabled(heap));  // a QSBR heap cannot be deleted (only destroyed)

  _mi_page_free_collect(page, true);  // collect cross-thread frees so `used` is precise
  _mi_page_qsbr_clear(page);          // deletion requires an unprotected heap; destruction requires no readers
  _mi_arenas_page_unabandon(page, NULL);

  if (mi_page_used(page)==0) {
    // free the page
    _mi_page_check_free_lists(page);
    _mi_arenas_page_free_direct(page, NULL);
  }
  else if (heap_target==NULL) {
    #if MI_GUARDED
    _mi_page_unguard_all(page);          // remove potential interior guard pages
    #endif
    // destroy the page
    _mi_page_free_collect(page, false); // collect the thread-free list first while `used` still counts those blocks (as `_mi_arenas_page_free_direct` collects it again) (see #1420)
    mi_page_used_reset(page);           // note: invariant `|local_free| + |free| == reserved - used`  does not hold in this case
    _mi_arenas_page_free_direct(page, NULL);
  }
  else {
    // move the page to `heap_target` as an abandoned page
    // first remove it from the current heap
    const size_t sbin = _mi_page_stats_bin(page);
    mi_arena_t* arena = NULL;
    size_t slice_index = 0;
    if (page->memid.memkind == MI_MEM_ARENA) {
      size_t slice_count;
      mi_arena_pages_t* arena_pages = NULL;
      arena = _mi_page_arena_pages(page, &slice_index, &slice_count, &arena_pages);
      mi_assert_internal(mi_bitmap_is_set(arena_pages->pages, slice_index));
      mi_bitmap_clear(arena_pages->pages, slice_index);
    }
    else {
      // os allocated
      mi_assert_internal(mi_memid_is_os(page->memid) && page->next == NULL);
      mi_atomic_decrement_relaxed(&heap->os_page_count);
      mi_atomic_increment_relaxed(&heap_target->os_page_count);
    }
    mi_heap_stat_decrease(heap, page_bins[sbin], 1);
    mi_heap_stat_decrease(heap, pages, 1);
    mi_theap_t* theap_target = info->theap_target;

    // and then add it to the new target heap
    if (arena != NULL) {
      mi_arena_pages_t* arena_pages_target = _mi_heap_ensure_arena_pages(heap_target, arena);
      if mi_unlikely(arena_pages_target==NULL) {
        // if we cannot allocate this, we move it to the main heap instead (which does not require allocation)
        heap_target = _mi_arena_heap_main(arena);
        theap_target = mi_heap_theap(heap_target);  // todo: find through theap_target tld?
        arena_pages_target = _mi_heap_ensure_arena_pages(heap_target, arena);
        mi_assert_internal(arena_pages_target!=NULL);
      }
      mi_assert_internal(mi_bitmap_is_clear(arena_pages_target->pages, slice_index));
      mi_bitmap_set(arena_pages_target->pages, slice_index);
    }
    page->heap = heap_target;
    #if MI_DEBUG>0
    mi_atomic_store_relaxed(&page->debug_fill_preserve, mi_atomic_load_relaxed(&heap_target->debug_fill_preserve));
    #endif
    mi_theap_stat_increase(theap_target, page_bins[sbin], 1);
    mi_theap_stat_increase(theap_target, pages, 1);

    // and abandon in the new heap
    _mi_arenas_page_abandon(page, theap_target);
  }
}

static bool mi_heap_move_page_at(size_t slice_index, size_t slice_count, mi_arena_t* arena, void* arg) {
  MI_UNUSED(slice_count);
  mi_heap_move_info_t* const info = (mi_heap_move_info_t*)arg;
  mi_bitmap_t* const pages = mi_heap_arena_pages(info->heap, arena)->pages;
  // temporarily clear the bit so a concurrent free cannot free the page while we claim it (see `_mi_arenas_page_free_prim`)
  if (!mi_bitmap_clear(pages, slice_index)) return true;  // freed concurrently
  mi_page_t* const page = mi_arena_page_at_slice(arena, slice_index);
  const bool claimed = mi_page_claim_ownership(page);
  mi_bitmap_set(pages, slice_index);
  if (claimed) { mi_heap_move_page(info, page); }
          else { info->retry = true; }  // owned by a concurrent free: retry in the next round
  return true;
}

static bool mi_heap_move_arena_pages(mi_arena_t* arena, const void* arg, void** result) {
  MI_UNUSED(result);
  mi_heap_move_info_t* const info = (mi_heap_move_info_t*)arg;
  mi_arena_pages_t* const arena_pages = mi_heap_arena_pages(info->heap, arena);
  if (arena_pages != NULL) {
    _mi_bitmap_forall_set(arena_pages->pages, &mi_heap_move_page_at, arena, info);
  }
  return true; // continue
}

// Claim an abandoned OS page that is not owned by a concurrent free.
// We claim under the lock: a free must unlink the page (under the lock) before it can free it.
static mi_page_t* mi_heap_os_abandoned_page_try_claim(mi_heap_move_info_t* info) {
  mi_heap_t* const heap = info->heap;
  mi_page_t* claimed = NULL;
  mi_lock(&heap->os_abandoned_pages_lock) {
    for (mi_page_t* page = heap->os_abandoned_pages; page != NULL && claimed == NULL; page = page->next) {
      if (mi_page_claim_ownership(page)) { claimed = page; }
                                    else { info->retry = true; }
    }
  }
  return claimed;
}

#if MI_DEBUG>1
static bool mi_heap_has_pages(mi_heap_t* heap) {
  for (size_t i = 0; i < MI_MAX_ARENAS; i++) {
    mi_arena_pages_t* const arena_pages = mi_atomic_load_ptr_relaxed(mi_arena_pages_t, &heap->arena_pages[i]);
    if (arena_pages != NULL && !mi_bitmap_is_all_clear(arena_pages->pages)) return true;
  }
  bool has_os_pages = false;
  mi_lock(&heap->os_abandoned_pages_lock) {
    has_os_pages = (heap->os_abandoned_pages != NULL);
  }
  return has_os_pages;
}
#endif

static void mi_heap_move_pages(mi_heap_t* heap, mi_heap_t* heap_target) {
  mi_assert_internal(heap->is_deleted && heap->theaps == NULL);
  #if MI_QSBR
  mi_assert_internal(mi_atomic_load_ptr_relaxed(mi_page_t, &heap->qsbr_delayed) == NULL);  // delayed pages are owned and cannot be claimed
  #endif
  mi_heap_move_info_t info = { heap, heap_target, (heap_target != NULL ? _mi_heap_theap(heap_target) : NULL), false };
  do {
    info.retry = false;
    mi_forall_arenas(heap, NULL, 0, &mi_heap_move_arena_pages, &info, NULL);
    mi_page_t* page;
    while ((page = mi_heap_os_abandoned_page_try_claim(&info)) != NULL) {
      mi_heap_move_page(&info, page);
    }
    if (mi_atomic_load_acquire(&heap->os_page_count) > 0) { info.retry = true; }  // a concurrent free of an unlinked OS page may still access the heap
    if (info.retry) { _mi_prim_thread_yield(); }  // some pages are still owned by concurrent frees
  } while (info.retry);
  #if MI_DEBUG>1
  mi_assert_internal(!mi_heap_has_pages(heap));
  for (size_t i = 0; i < MI_ARENA_BIN_COUNT; i++) {
    mi_assert_internal(mi_atomic_load_relaxed(&heap->abandoned_count[i])==0);
  }
  #endif
}

void _mi_heap_move_pages(mi_heap_t* heap_from, mi_heap_t* heap_to) {
  if (mi_is_heap_main(heap_from)) return;
  if (heap_to==NULL) { heap_to = heap_from->subproc->heap_main; }
  mi_heap_move_pages(heap_from, heap_to);
}

void _mi_heap_destroy_pages(mi_heap_t* heap_from) {
  if (mi_is_heap_main(heap_from)) return;
  mi_heap_qsbr_collect_delayed(heap_from, SIZE_MAX, true); // Destruction requires external exclusion, so no delayed-page collector can hold the lock or detached pages.
  mi_heap_move_pages(heap_from, NULL);
}


/* -----------------------------------------------------------
  QSBR: heap delayed singleton pages and abandoned page collection
----------------------------------------------------------- */

#if MI_QSBR
// Append a list of pages at the end of the delayed list (with the delayed lock held).
static void mi_heap_qsbr_delayed_append(mi_heap_t* heap, mi_page_t* first, mi_page_t* last) {
  mi_assert_internal(first != NULL && last != NULL && last->next == NULL);
  if (heap->qsbr_delayed_last == NULL) {
    mi_atomic_store_ptr_relaxed(mi_page_t, &heap->qsbr_delayed, first);
  }
  else {
    heap->qsbr_delayed_last->next = first;
  }
  heap->qsbr_delayed_last = last;
}

// The caller removed the page from its old queues/registries and keeps ownership.
void _mi_heap_qsbr_delayed_enqueue(mi_page_t* page) {
  mi_assert_internal(mi_page_is_owned(page) && mi_page_is_abandoned(page));
  mi_assert_internal(mi_page_all_free(page) && mi_page_is_singleton(page));
  mi_assert_internal(!mi_page_is_abandoned_mapped(page));
  mi_assert_internal(page->next == NULL && page->prev == NULL);
  mi_assert_internal(!mi_page_qsbr_is_in_queue(page) && page->qsbr_next == NULL && page->qsbr_prev == NULL);
  mi_assert_internal(!mi_page_qsbr_is_clear(&page->qsbr));
  mi_heap_t* heap = mi_page_heap(page);
  mi_assert_internal(mi_heap_qsbr_is_enabled(heap));
  _mi_page_update_stats(page);
  mi_lock(&heap->qsbr_delayed_lock) {
    mi_heap_qsbr_delayed_append(heap, page, page);
  }
}

// Detach up to `max_pages` (at least 1) from the front of the delayed list (the oldest pages), free those whose
// grace period completed, and append the others at the end again (so they rotate). The lock is only held to
// update the list; polling QSBR (which calls into the embedder) and freeing pages happens outside the lock.
static void mi_heap_qsbr_collect_delayed(mi_heap_t* heap, size_t max_pages, bool destroy) {
  if (mi_atomic_load_ptr_relaxed(mi_page_t, &heap->qsbr_delayed) == NULL) return;
  if (!mi_lock_try_acquire(&heap->qsbr_delayed_lock)) return;
  mi_page_t* const first = mi_atomic_load_ptr_relaxed(mi_page_t, &heap->qsbr_delayed);
  if (first == NULL) {
    mi_lock_release(&heap->qsbr_delayed_lock);
    return;
  }
  mi_page_t* last = first;
  while (--max_pages > 0 && last->next != NULL) { last = last->next; }
  mi_atomic_store_ptr_relaxed(mi_page_t, &heap->qsbr_delayed, last->next);
  if (last->next == NULL) { heap->qsbr_delayed_last = NULL; }
  last->next = NULL;
  mi_lock_release(&heap->qsbr_delayed_lock);

  mi_page_t* delayed_first = NULL;
  mi_page_t* delayed_last = NULL;
  mi_page_t* next;
  for (mi_page_t* page = first; page != NULL; page = next) {
    next = page->next;
    page->next = NULL;
    if (destroy || !_mi_page_qsbr_should_delay(page)) {
      _mi_page_qsbr_clear(page);
      _mi_arenas_page_free_direct(page, NULL);
    }
    else {
      if (delayed_last == NULL) { delayed_first = page; }
      else { delayed_last->next = page; }
      delayed_last = page;
    }
  }
  if (delayed_first != NULL) {
    mi_lock(&heap->qsbr_delayed_lock) {
      mi_heap_qsbr_delayed_append(heap, delayed_first, delayed_last);
    }
  }
}

void _mi_heap_qsbr_collect_delayed(mi_heap_t* heap) {
  mi_heap_qsbr_collect_delayed(heap, 8, false);
}

// Collect an abandoned page that we claimed: free it if it is empty and safe, and otherwise unown it again.
static void mi_heap_collect_abandoned_page(mi_page_t* page) {
  mi_assert_internal(mi_page_is_owned(page));
  _mi_page_free_collect(page, true);
  if (mi_page_all_free(page) && !_mi_page_qsbr_should_delay(page)) {
    _mi_arenas_page_unabandon(page, NULL);
    _mi_arenas_page_free(page, NULL);  // frees the page
  }
  else {
    _mi_abandoned_page_unown(page, NULL);  // leaves a delayed page in place (but an empty singleton goes to `_mi_arenas_page_free`)
  }
}

// Pages owned by a concurrent free are skipped (as that free collects the page itself).
static bool mi_heap_collect_abandoned_page_at(size_t slice_index, size_t slice_count, mi_arena_t* arena, void* arg) {
  MI_UNUSED(slice_count);
  mi_heap_t* const heap = (mi_heap_t*)arg;
  mi_bitmap_t* const pages = mi_heap_arena_pages(heap, arena)->pages;
  // temporarily clear the bit so a concurrent free cannot free the page while we claim it (see `_mi_arenas_page_free_prim`)
  if (!mi_bitmap_clear(pages, slice_index)) return true;  // freed concurrently
  mi_page_t* const page = mi_arena_page_at_slice(arena, slice_index);
  const bool claimed = (mi_page_is_abandoned(page) && mi_page_claim_ownership(page));
  mi_bitmap_set(pages, slice_index);
  if (claimed) { mi_heap_collect_abandoned_page(page); }
  return true;
}

static bool mi_heap_collect_abandoned_arena(mi_arena_t* arena, const void* arg, void** result) {
  MI_UNUSED(result);
  mi_heap_t* heap = (mi_heap_t*)arg;
  mi_arena_pages_t* arena_pages = mi_heap_arena_pages(heap, arena);
  if (arena_pages == NULL) return true;
  return _mi_bitmap_forall_set(arena_pages->pages, &mi_heap_collect_abandoned_page_at, arena, heap);
}

// Claim the first abandoned OS page after `page` (or from the start if `page==NULL`).
// The `page` must be owned by us so it stays in the list (and cannot be freed concurrently).
static mi_page_t* mi_heap_os_abandoned_page_claim_next(mi_heap_t* heap, mi_page_t* page) {
  mi_page_t* claimed = NULL;
  mi_lock(&heap->os_abandoned_pages_lock) {
    for (mi_page_t* p = (page==NULL ? heap->os_abandoned_pages : page->next); p != NULL && claimed == NULL; p = p->next) {
      if (mi_page_claim_ownership(p)) { claimed = p; }
    }
  }
  return claimed;
}

// Collect the abandoned pages of a heap; this can run concurrently with frees (but not with allocations, see `doc/qsbr.md`).
void _mi_heap_collect_abandoned(mi_heap_t* heap) {
  mi_assert_internal(heap != NULL);
  mi_lock(&heap->theaps_lock) {
    // Use the all-pages map: abandoned size-class maps exclude singletons and full pages.
    mi_forall_arenas(heap, NULL, 0, &mi_heap_collect_abandoned_arena, heap, NULL);
    mi_page_t* page = mi_heap_os_abandoned_page_claim_next(heap, NULL);
    while (page != NULL) {
      mi_page_t* const next = mi_heap_os_abandoned_page_claim_next(heap, page);  // claim while we still own `page`
      mi_heap_collect_abandoned_page(page);
      page = next;
    }
    mi_heap_qsbr_collect_delayed(heap, SIZE_MAX, false);
  }
}
#else
static void mi_heap_qsbr_collect_delayed(mi_heap_t* heap, size_t max_pages, bool destroy) {
  MI_UNUSED(heap); MI_UNUSED(max_pages); MI_UNUSED(destroy);
  return;
}
void _mi_heap_qsbr_collect_delayed(mi_heap_t* heap) {
  MI_UNUSED(heap);
  return;
}
void _mi_heap_collect_abandoned(mi_heap_t* heap) {
  MI_UNUSED(heap);
  return;
}
#endif
