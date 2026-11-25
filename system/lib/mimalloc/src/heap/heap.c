/*----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/prim/prim-tls.h"   // _mi_heap_theap
#include "mimalloc/util/stats.h"
#include "mimalloc/alloc/free.h"      // mi_free_subproc_safe
#include "mimalloc/alloc/page.h"
#include "mimalloc/arena/arena-page.h"
#include "mimalloc/arena/arena.h"
#include "mimalloc/heap/heap.h"


/* -----------------------------------------------------------
  Heap's
----------------------------------------------------------- */

mi_decl_cold bool _mi_heap_invalid(const mi_heap_t* heap) {
  _mi_error_message(EFAULT, "invalid heap handle %p (the heap was deleted or the handle was corrupted)\n", heap);
  return false;
}

mi_theap_t* mi_heap_theap(mi_heap_t* heap) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  return _mi_heap_theap(heap);  // in prim/prim.h
}

void mi_heap_set_numa_affinity(mi_heap_t* heap, int numa_node) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return;
  heap->numa_node = (numa_node < 0 ? -1 : numa_node % _mi_os_numa_node_count());
}

void mi_heap_set_debug_fill_preserve(mi_heap_t* heap, size_t preserve) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return;
  if (preserve > MI_MiB) { preserve = MI_MiB; }  // fills are limited to 1 MiB anyway
  mi_atomic_store_relaxed(&heap->debug_fill_preserve, preserve);
}

void mi_heap_stats_merge_to_subproc(mi_heap_t* heap) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return;
  _mi_stats_merge_into(&heap->subproc->stats, &heap->stats);
}

void mi_heap_stats_merge_to_main(mi_heap_t* heap) {
  mi_assert_internal(heap!=NULL && !mi_is_heap_main(heap));
  _mi_stats_merge_into(&mi_heap_get_heap_main(heap)->stats, &heap->stats);
}

bool _mi_heap_theap_set(mi_heap_t* heap, mi_theap_t* theap) {
  mi_assert_internal(mi_addr(theap) == 1 || mi_theap_heap(theap)==heap);
  mi_assert_internal(!_mi_is_empty_theap(theap));
  mi_assert_internal(heap->theap != 0);
  return _mi_thread_local_set(heap->theap,theap);
}

// mi_theap_t* _mi_heap_theap_get_peek(const mi_heap_t* heap) {
//   mi_theap_t* theap;
//   mi_assert_internal(heap->theap != 0);
//   if mi_likely(heap->theap!=0) {  // paranoia
//     theap = (mi_theap_t*)_mi_thread_local_get(heap->theap);
//   }
//   else {
//     _mi_error_message(EFAULT, "no thread-local reserved for heap (%p)\n", heap);
//     return NULL;
//   }
//   mi_assert_internal(!_mi_is_empty_theap(theap));
//   mi_assert_internal(theap->heap == heap);  // this goes wrong if using main heaps across subprocesses (as all share the same key)
//   return theap;
// }


static mi_decl_noinline mi_theap_t* mi_heap_init_theap(const mi_heap_t* const_heap)
{
  mi_heap_t* heap = (mi_heap_t*)const_heap;
  mi_assert_internal(heap!=NULL);

  // initialize thread first in case this is the main heap
  // (which may allocate the default theap already for the main heap)
  if (!_mi_thread_is_initialized()) {
    if (_mi_thread_init() == NULL) return NULL;
  }

  // get the thread local theap
  mi_theap_t* theap = (mi_theap_t*)_mi_thread_local_get(heap->theap);

  // create a fresh theap?
  if (theap==NULL) {
    // allocate a fresh theap (fails on out-of-memory or if the heap is deleted)
    theap = _mi_theap_create(heap, mi_theap_get_default()->tld); // sets the theap thread local
    if (theap==NULL) return NULL;
    _mi_heap_theap_set(heap, theap);
    mi_assert_internal(theap == (mi_theap_t*)_mi_thread_local_get(heap->theap));
  }
  return theap;
}


// a theap for which allocation returns NULL (see `alloc/alloc-generic.c:mi_theap_init`)
// note: on a done thread the unavailable theap falls back to meta allocation so we return the empty theap instead
static mi_theap_t* mi_theap_failed(void) {
  return (_mi_thread_is_done() ? _mi_theap_empty_get() : _mi_theap_unavailable());
}

// get (and possibly create) the theap belonging to a heap
mi_decl_cold mi_theap_t* _mi_heap_theap_get_or_init(const mi_heap_t* heap)
{
  if (!mi_heap_handle_is_valid(heap)) { return mi_theap_failed(); }  // allocation returns NULL
  mi_assert_internal(heap->theap != 0 || heap->is_deleted);  // the slot is released by `mi_heap_free`
  _mi_theap_tls_check();  // report if the cached theap was overwritten
  mi_theap_t* theap = (mi_theap_t*)_mi_thread_local_get(heap->theap);
  if mi_unlikely(theap==NULL) {
    if (_mi_thread_is_done()) {
      // don't re-initialize; allocation in the main heap uses the meta-data theap while other heaps return NULL (see `alloc/alloc-generic.c`)
      return (mi_addr(heap) == mi_addr(mi_heap_main()) ? _mi_theap_unavailable() : mi_theap_failed());
    }
    theap = mi_heap_init_theap(heap);
    if (theap==NULL) { return mi_theap_failed(); }
  }
  else if mi_unlikely(_mi_theap_is_unavailable(theap)) {
    return mi_theap_failed();  // released by this thread (see `mi_thread_heap_release`)
  }
  _mi_theap_cached_set(theap);
  return theap;
}

void _mi_heap_init(mi_heap_t* heap, mi_thread_local_t theap_slot, mi_subproc_t* subproc, mi_arena_id_t exclusive_arena_id, bool use_qsbr)
{
  mi_assert_internal(_mi_bin_size(_mi_bin(MI_LARGE_MAX_OBJ_SIZE)) == MI_LARGE_MAX_OBJ_SIZE);  // must be a bin size
  // init fields
  heap->cookie = mi_heap_cookie(heap);
  heap->theap = theap_slot;
  heap->subproc = subproc;
  heap->heap_seq = mi_atomic_increment_relaxed(&subproc->heap_total_count);
  heap->exclusive_arena = _mi_arena_from_id(exclusive_arena_id);
  heap->reclaim_on_free = mi_option_get(mi_option_page_reclaim_on_free);
  heap->numa_node = -1; // no initial affinity
  heap->is_deleted = false;
  #if MI_QSBR
  heap->qsbr_enabled = use_qsbr;
  mi_atomic_store_ptr_relaxed(mi_page_t, &heap->qsbr_delayed, NULL);
  heap->qsbr_delayed_last = NULL;
  mi_lock_init(&heap->qsbr_delayed_lock);
  if (use_qsbr) {
    if (heap->reclaim_on_free > 0) { heap->reclaim_on_free = 0; }
    mi_heap_qsbr_init(&heap->qsbr);
  }
  #else
  mi_assert_internal(!use_qsbr);
  MI_UNUSED(use_qsbr);
  #endif
  mi_atomic_store_ptr_relaxed(mi_profiler_t, &heap->profiler, NULL);  // set when the heap is linked into the subprocess
  mi_stats_header_init(&heap->stats);
  mi_lock_init(&heap->theaps_lock);
  mi_lock_init(&heap->os_abandoned_pages_lock);
  mi_lock_init(&heap->arena_pages_lock);

  // push onto the subproc heaps
  mi_lock(&heap->subproc->heaps_lock) {
    // inherit the profiler under the lock so a concurrent `mi_subproc_profile` either sees this heap or we see its profiler
    mi_atomic_store_ptr_relaxed(mi_profiler_t, &heap->profiler, mi_atomic_load_ptr_acquire(mi_profiler_t,&subproc->profiler));
    mi_heap_t* head = heap->subproc->heaps;
    heap->prev = NULL;
    heap->next = head;
    if (head!=NULL) { head->prev = heap;  }
    heap->subproc->heaps = heap;
  }
  mi_atomic_increment_relaxed(&subproc->heap_count);
  mi_subproc_stat_increase(subproc, heaps, 1);
  mi_assert_internal(mi_is_heap_main(heap) ? heap->theap == mi_thread_local_key_fast : heap->theap != 0);
}

mi_heap_t* _mi_heap_new_for_subproc(mi_subproc_t* subproc, mi_arena_id_t exclusive_arena_id, bool is_main_heap, bool use_qsbr) {
  mi_assert_internal(is_main_heap ? (subproc->heap_main == NULL && !_mi_subproc_is_main(subproc)) : subproc->heap_main != NULL);
  mi_heap_t* heap;
  if (is_main_heap) {
    // the main heap of a subprocess is allocated in the main subprocess as that outlives all other subprocesses
    // (and we use the detached meta theap as the current thread may belong to another subprocess)
    heap = (mi_heap_t*)_mi_meta_zalloc(_mi_subproc_main(), sizeof(mi_heap_t), NULL);
  }
  else {
    #if MI_SECURE
    // in secure mode, keep heap data separate from user data (at the cost of taking a lock)
    heap = (mi_heap_t*)_mi_meta_zalloc(subproc, sizeof(mi_heap_t), NULL);
    #else
    heap = (mi_heap_t*)mi_heap_zalloc(subproc->heap_main, sizeof(mi_heap_t));
    #endif
  }
  if (heap==NULL) return NULL;
  // reserve a thread local slot for this heap (see also issue #1230)
  mi_thread_local_t theap_slot = (is_main_heap ? mi_thread_local_key_fast : _mi_thread_local_create());
  if (theap_slot == 0) {
    _mi_error_message(EFAULT, "unable to dynamically create a thread local for a heap\n");
    mi_free(heap);
    return NULL;
  }
  if (is_main_heap) {
    mi_assert_internal(subproc->heap_main == NULL);
    subproc->heap_main = heap;
  }
  _mi_heap_init(heap, theap_slot, subproc, exclusive_arena_id, use_qsbr);
  return heap;
}

mi_heap_t* mi_heap_new_in_arena(mi_arena_id_t exclusive_arena_id) {
  // `mi_heap_new` may be the very first mimalloc call in a process, in which case the
  // main heap does not exist yet and `_mi_heap_new_for_subproc` would allocate from a NULL `subproc->heap_main`.
  mi_thread_init();
  return _mi_heap_new_for_subproc(_mi_subproc(), exclusive_arena_id, false, false);
}

mi_heap_t* mi_heap_new(void) {
  return mi_heap_new_in_arena(0);
}

mi_heap_t* mi_heap_new_qsbr(void) {
  #if MI_QSBR
  mi_thread_init();
  return _mi_heap_new_for_subproc(_mi_subproc(), 0, false, true);
  #else
  _mi_error_message(ENOSYS, "mi_heap_new_qsbr requires a build with QSBR support\n");
  return NULL;
  #endif
}

// free all theaps belonging to this heap (without deleting their pages as we do this arena wise for efficiency)
static void mi_heap_free_theaps(mi_heap_t* heap) {
  // This can run concurrently with a thread that terminates (see `heap/process.c:mi_thread_theaps_done`),
  // and we need to ensure we free theaps atomically.

  // From now on no new theaps can be attached to this heap (see `heap/theap.c:mi_theap_attach_to_heap`)
  mi_lock(&heap->theaps_lock) {
    heap->is_deleted = true;
  }

  // We first detach our theaps list from any thread local lists
  _mi_heap_detach_theaps(heap);

  // clear our own thread local slot so `_mi_page_associated_theap` does not read a freed theap (through destroy pages -> page_update_stats).
  // (the key itself is only freed in `mi_heap_free` as concurrent frees may still read it until all pages are gone)
  // (main heaps of all subprocesses share the fast key which must never be released)
  if (!mi_is_heap_main(heap) && _mi_thread_local_get(heap->theap) != NULL) {
    _mi_thread_local_set(heap->theap, NULL);
  }

  // Now we can safely free the theaps
  mi_lock(&heap->theaps_lock) { // paranoia
    mi_theap_t* theap = heap->theaps;
    heap->theaps = NULL;
    while(theap != NULL) {
      mi_theap_t* next = theap->hnext;
      theap->hnext = NULL;
      theap->hprev = NULL;
      mi_assert_internal(theap->tld==NULL);
      // abandon any owned pages into the heap-wide registries so heap delete/destroy can visit them
      _mi_theap_abandon_owned_pages(theap);
      // clear the heap so a stale `_mi_theap_cached` entry in another thread no longer matches
      // (a new heap can be allocated at the same address and would otherwise pick up this freed theap)
      mi_atomic_store_ptr_release(mi_heap_t, &theap->heap, NULL);
      // merge stats into the owning heap stats
      _mi_stats_merge_into(&heap->stats, &theap->stats);
      // and free
      _mi_theap_decref(theap);  // a cached entry can still point to the theap
      theap = next;
    }
  }
}

// free the heap resources (assuming the pages are already moved/destroyed, and all theaps have been freed)
static void mi_heap_free(mi_heap_t* heap, bool acquire_heaps_lock) {
  mi_assert_internal(heap!=NULL); // && !mi_is_process_heap_main(heap));

  // free all arena pages infos
  const bool is_main = mi_is_heap_main(heap);
  if (!is_main) {  // pages for the main heap are pre-allocated in the arenas
    mi_lock(&heap->arena_pages_lock) {
      for (size_t i = 0; i < MI_MAX_ARENAS; i++) {
        mi_arena_pages_t* arena_pages = mi_atomic_load_ptr_relaxed(mi_arena_pages_t, &heap->arena_pages[i]);
        if (arena_pages!=NULL) {
          mi_atomic_store_ptr_relaxed(mi_arena_pages_t, &heap->arena_pages[i], NULL);
          _mi_free_subproc_safe(arena_pages);
        }
      }
    }
  }

  // remove the heap from the subproc
  if (!is_main) {
    mi_heap_stats_merge_to_main(heap);
  }
  else {
    _mi_stats_merge_into(&heap->subproc->stats,&heap->stats);
  }
  mi_atomic_decrement_relaxed(&heap->subproc->heap_count);
  mi_subproc_stat_decrease(heap->subproc, heaps, 1);
  mi_lock_maybe(&heap->subproc->heaps_lock, acquire_heaps_lock) {
    mi_list_unlink_check(heap, prev, next, heap->subproc->heaps, NULL);
    if (heap->next!=NULL) { heap->next->prev = heap->prev; }
    if (heap->prev!=NULL) { heap->prev->next = heap->next; }
                     else { heap->subproc->heaps = heap->next; }
  }

  mi_lock_done(&heap->theaps_lock);
  mi_lock_done(&heap->os_abandoned_pages_lock);
  mi_lock_done(&heap->arena_pages_lock);
  #if MI_QSBR
  mi_assert_internal(mi_atomic_load_ptr_relaxed(mi_page_t, &heap->qsbr_delayed) == NULL);
  mi_lock_done(&heap->qsbr_delayed_lock);
  #endif
  if (!is_main) {  // main heaps of all subprocesses share the fast key which must never be released
    _mi_thread_local_free(heap->theap);
    heap->theap = 0;
  }
  if (!mi_is_process_heap_main(heap)) {
    heap->cookie = 0;  // invalidate the handle
    _mi_free_subproc_safe(heap);
  }
}

// Release the theap of the calling thread for this heap: its pages are abandoned (so blocks can still be freed from any thread),
// and the thread-local slot is set to the unavailable theap so this thread can no longer allocate in the heap (and no free path recreates a theap).
void mi_thread_heap_release(mi_heap_t* heap) mi_attr_noexcept {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return;
  if (mi_is_heap_main(heap)) {
    _mi_warning_message("cannot release the main heap\n");
    return;
  }
  if (_mi_thread_is_done()) return;   // all theaps of this thread are already released
  mi_theap_t* const theap = (mi_theap_t*)_mi_thread_local_get(heap->theap);
  if (_mi_theap_is_unavailable(theap)) return;   // already released
  if (theap!=NULL) {
    _mi_theap_release(theap, _mi_theap_unavailable());
  }
  else {
    // never used in this thread: still prevent a theap being created later on
    mi_thread_init();  // so the thread locals are released at thread exit
    _mi_thread_local_set(heap->theap, _mi_theap_unavailable());
  }
}

#if MI_DEBUG
// Report (verbose) if some threads still own a theap of a heap that is deleted (see `mi_thread_heap_release`)
static void mi_heap_check_released(mi_heap_t* heap) {
  size_t count = 0;
  mi_lock(&heap->theaps_lock) {
    for (mi_theap_t* theap = heap->theaps; theap != NULL; theap = theap->hnext) { count++; }
  }
  if (count > 0) {
    _mi_verbose_message("deleting heap %p while %zu thread(s) did not release it first (see mi_thread_heap_release)\n", heap, count);
  }
}
#endif

void mi_heap_delete(mi_heap_t* heap) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return;
  mi_heap_t* heap_main = mi_heap_get_heap_main(heap);
  if (heap == heap_main) {
    _mi_warning_message("cannot delete the main heap\n");
    return;
  }
  if (mi_heap_qsbr_is_enabled(heap)) {
    _mi_warning_message("cannot delete a QSBR heap; use destroy after readers and mutators have stopped\n");
    return;
  }
  // release our own theap first (the thread-local key is freed in `mi_heap_free`)
  _mi_theap_release(_mi_heap_theap_peek(heap), NULL);
  #if MI_DEBUG
  mi_heap_check_released(heap);
  #endif
  mi_heap_free_theaps(heap);
  _mi_heap_move_pages(heap, heap_main);
  mi_heap_free(heap,true /* acquire subproc->heaps_lock */);
}

void _mi_heap_force_destroy(mi_heap_t* heap, bool acquire_heaps_lock) {
  if (heap==NULL) return;
  mi_heap_free_theaps(heap);
  _mi_heap_destroy_pages(heap);
  // if (_mi_subproc_main()->heap_main == heap) {
  //   _mi_stats_merge_into(&heap->subproc->stats,&heap->stats);
  // }
  // else
  {
    mi_heap_free(heap, acquire_heaps_lock);   // todo: release locks of the main heap?
  }
}

void mi_heap_destroy(mi_heap_t* heap) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return;
  if (mi_is_heap_main(heap)) {
    _mi_warning_message("cannot destroy the main heap\n");
    return;
  }
  _mi_heap_force_destroy(heap,true /* acquire subproc->heaps_lock */);
}

mi_heap_t* mi_heap_of(const void* p) {
  mi_page_t* const page = _mi_safe_ptr_page(p);
  if (page==NULL) return NULL;
  return mi_page_heap(page);
}

bool mi_any_heap_contains(const void* p) {
  mi_page_t* const page = _mi_safe_ptr_page(p);
  return (page!=NULL);
}

bool mi_heap_contains(const mi_heap_t* heap, const void* p) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return false;
  return (heap==mi_heap_of(p));
}

// deprecated
bool mi_check_owned(const void* p) {
  return mi_any_heap_contains(p);
}

// unsafe heap utilization function for DragonFly (see issue #1258)
// If the page of pointer `p` belongs to `heap` (or `heap==NULL`) and has less than `perc_threshold` used blocks in its used area return `true`.
// This function is unsafe in general as it assumes we are the only thread accessing the page of `p`.
bool mi_unsafe_heap_page_is_under_utilized(mi_heap_t* heap, void* p, size_t perc_threshold) mi_attr_noexcept {
  if (p==NULL) return false;
  const mi_page_t* const page = _mi_safe_ptr_page(p);   // Get the page containing this pointer
  if (page==NULL || mi_page_used(page)==page->capacity || page->capacity < page->reserved) return false;
  // If the page is the head of the queue, it is currently being used for
  // allocations; we skip it to avoid immediate thrashing.
  if (page->prev == NULL)  return false;

  // match heap?
  const mi_heap_t* const page_heap = mi_page_heap(page);
  if (page_heap==NULL) return false;
  if (heap!=NULL && page_heap!=heap) return false;

  // check utilization
  if (page->capacity==0)   return false;
  if (perc_threshold>=100) return true;
  return (perc_threshold >= ((100UL*mi_page_used(page)) / page->capacity));
}
