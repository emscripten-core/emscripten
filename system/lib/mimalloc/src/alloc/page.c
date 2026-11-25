/*----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/types.h"
#include "mimalloc/prim/prim-tls.h"
#include "mimalloc/prim/prim-mte.h"  // mi_memtag_load, mi_memtag_blocks_init, mi_ptr_memtag
#include "mimalloc/util/random.h"
#include "mimalloc/util/stats.h"
#include "mimalloc/arena/arena-page.h"
#include "mimalloc/alloc/page.h"
#include "mimalloc/profile/sample-profile.h"
#include "page-queue.h"
#include "page.h"


/* -----------------------------------------------------------
  Page invariants
----------------------------------------------------------- */

// Index a block in a page (with its memory tag if `memtag` is set)
static inline mi_block_t* mi_page_block_at(const mi_page_t* page, size_t block_size, size_t i, bool memtag) {
  mi_assert_internal(page != NULL);
  mi_assert_internal(i <= page->reserved);
  mi_block_t* const block = (mi_block_t*)(mi_page_start(page) + (i * block_size));
  return (memtag ? (mi_block_t*)mi_memtag_load(block) : block);
}

// The block following `block` (with its memory tag if `memtag` is set)
static inline mi_block_t* mi_block_after(mi_block_t* block, size_t block_size, bool memtag) {
  mi_block_t* const next = (mi_block_t*)((uint8_t*)block + block_size);
  return (memtag ? (mi_block_t*)mi_memtag_load(next) : next);
}

#if (MI_DEBUG>=3)
static size_t mi_page_list_count(mi_page_t* page, mi_block_t* head) {
  mi_assert_internal(mi_ptr_page(mi_page_start(page)) == page);
  const uint8_t* slice_start = mi_page_slice_start(page);
  mi_assert_internal(mi_is_aligned(slice_start,MI_PAGE_ALIGN));
  size_t count = 0;
  while (head != NULL) {
    mi_assert_internal((uint8_t*)head - slice_start > (ptrdiff_t)MI_LARGE_PAGE_SIZE || page == mi_ptr_page(head));
    count++;
    head = mi_block_next(page, head);
  }
  return count;
}

/*
// Start of the page available memory
static inline uint8_t* mi_page_area(const mi_page_t* page) {
  return _mi_page_start(_mi_page_segment(page), page, NULL);
}
*/

static bool mi_page_list_is_valid(mi_page_t* page, mi_block_t* p) {
  size_t psize;
  uint8_t* page_area = mi_page_area(page, &psize);
  const mi_addr_t start = mi_addr(page_area);
  const mi_addr_t end   = start + psize;
  while(p != NULL) {
    if (mi_addr(p) < start || mi_addr(p) >= end) return false;  // compare without memory tags
    p = mi_block_next(page, p);
  }
#if MI_DEBUG>3 // generally too expensive to check this
  if (page->free_is_zero) {
    const size_t ubsize = mi_page_usable_block_size(page);
    for (mi_block_t* block = page->free; block != NULL; block = mi_block_next(page, block)) {
      mi_assert_expensive(mi_mem_is_zero(block + 1, ubsize - sizeof(mi_block_t)));
    }
  }
#endif
  return true;
}

static bool mi_page_is_valid_init(mi_page_t* page) {
  mi_assert_internal(mi_page_block_size(page) > 0);
  mi_assert_internal(mi_page_used(page) <= page->capacity);
  mi_assert_internal(page->capacity <= page->reserved);
  #if MI_FREE_IS_CHECKED
  const size_t capacity_size = mi_page_capacity_size(page);
  mi_assert_internal(page->capacity == capacity_size / mi_page_block_size(page));
  mi_assert_internal(capacity_size % mi_page_block_size(page) == 0);
  MI_UNUSED(capacity_size);
  #endif

  mi_assert_internal(page->heap!=NULL);
  mi_theap_t* const page_theap = _mi_heap_theap_peek(page->heap);
  mi_assert_internal(page_theap == NULL || mi_page_theap(page)==page_theap ||
                     mi_page_theap(page)->tld == NULL || mi_page_theap(page)->tld->thread_id == MI_THREAD_ID_DETACHED);

  // const size_t bsize = mi_page_block_size(page);
  // uint8_t* start = mi_page_start(page);
  //mi_assert_internal(start + page->capacity*page->block_size == page->top);

  mi_assert_internal(mi_page_list_is_valid(page,page->free));
  mi_assert_internal(mi_page_list_is_valid(page,page->local_free));

  #if MI_DEBUG>3 // generally too expensive to check this
  if (page->free_is_zero) {
    const size_t ubsize = mi_page_usable_block_size(page);
    for(mi_block_t* block = page->free; block != NULL; block = mi_block_next(page,block)) {
      mi_assert_expensive(mi_mem_is_zero(block + 1, ubsize - sizeof(mi_block_t)));
    }
  }
  #endif

  #if !MI_TRACK_ENABLED && !MI_TSAN
  mi_block_t* tfree = mi_page_thread_free(page);
  mi_assert_internal(mi_page_list_is_valid(page, tfree));
  //size_t tfree_count = mi_page_list_count(page, tfree);
  //mi_assert_internal(tfree_count <= page->thread_freed + 1);
  #endif

  size_t free_count = mi_page_list_count(page, page->free) + mi_page_list_count(page, page->local_free);
  mi_assert_internal(mi_page_used(page) + free_count == page->capacity);

  mi_assert_internal(mi_page_alloc_count(page) + mi_page_last_used(page) >= mi_page_used(page));
  return true;
}

bool _mi_page_is_valid(mi_page_t* page) {
  mi_assert_internal(mi_page_is_valid_init(page));
  #if MI_SECURE
  mi_assert_internal(page->keys[0] != 0);
  #endif
  if (!mi_page_is_abandoned(page)) {
    //mi_assert_internal(!_mi_process.process_is_initialized);
    mi_assert_internal(page->heap!=NULL);
    mi_theap_t* const page_theap = _mi_heap_theap_peek(page->heap);
    mi_assert_internal(page_theap == NULL || mi_page_theap(page)==page_theap ||
                       mi_page_theap(page)->tld == NULL || mi_page_theap(page)->tld->thread_id == MI_THREAD_ID_DETACHED);
    {
      mi_page_queue_t* pq = mi_page_queue_of(page);
      mi_assert_internal(mi_page_queue_contains(pq, page));
      mi_assert_internal(pq->block_size==mi_page_block_size(page) || mi_page_is_huge(page) || mi_page_is_in_full(page));
      // mi_assert_internal(mi_theap_contains_queue(mi_page_theap(page),pq));
    }
  }
  mi_assert_internal(mi_page_alloc_count(page) + mi_page_last_used(page) >= mi_page_used(page));
  mi_assert_internal(mi_page_alloc_count(page) >= mi_page_last_alloc(page));
  return true;
}
#endif

/* -----------------------------------------------------------
  Page statistics
----------------------------------------------------------- */

// Reset the `alloc_count` (and `last_alloc`), and set `last_used` to `used`
static void mi_page_reset_alloc_count(mi_page_t* page) {
  const size_t used = mi_page_used(page);
  #if MI_SIZE_SIZE >= 8
  page->xused.used_alloc = (used << 32) | used;
  #else
  page->xused.used_alloc = used;
  page->xlast_used = (uint16_t)used;
  page->xlast_alloc = 0;
  #endif
  mi_assert_internal(mi_page_alloc_count(page) + mi_page_last_used(page) >= mi_page_used(page));
  mi_assert_internal(mi_page_alloc_count(page) >= mi_page_last_alloc(page));
}

#if MI_STATS || (MI_SAMPLE==1)
// Gets the theap belonging to a page.
static mi_theap_t* mi_theap_of_page(mi_page_t* page) {
  mi_theap_t* theap = page->theap;
  if mi_unlikely(mi_page_thread_id(page) != _mi_prim_thread_id()) {
    theap = _mi_page_associated_theap_peek(page);
  }
  return (theap != NULL && theap->tld != NULL ? theap : NULL);
}
#endif

#if MI_SAMPLE==1 /* for ==2, the countdown is already done at every `alloc/alloc.c:mi_page_alloc_zero` */

// Adjust the theap->sample_countdown based on the current page allocation count
// in the `mi_malloc_generic` function this is countdown is checked
static void mi_theap_adjust_sample_countdown(mi_theap_t* theap, mi_page_t* page, size_t alloc_count)
{
  mi_assert_internal(mi_page_alloc_count(page) + mi_page_last_used(page) >= mi_page_used(page));
  mi_assert_internal(theap!=NULL);
  if (theap->sample_rate==0) return;

  const size_t last_alloc = mi_page_last_alloc(page);
  if (alloc_count <= last_alloc) return;

  // update countdown
  const size_t bsize = mi_page_usable_block_size(page);  // (already excludes the padding)
  const size_t alloc_diff = alloc_count - last_alloc;
  const uint64_t requested = (uint64_t)alloc_diff * (uint64_t)bsize;
  mi_theap_sample_countdown_sub(theap, requested);
}
#endif

#if MI_STATS
// Merge stats from the page into the corresponding theap (or heap if theapx==NULL).
static void mi_theap_page_merge_stats(mi_theap_t* theapx, const mi_page_t* page, size_t alloc_count, size_t free_count ) {
  // get heap (as the theap might be NULL)
  mi_heap_t* const heap = mi_page_heap(page);
  mi_assert_internal(theapx == NULL || (theapx->tld != NULL && _mi_thread_id() == theapx->tld->thread_id));
  mi_theapx_stat_counter_increase(heap,theapx,pages_stat_updates,1);
  mi_theapx_stat_counter_increase(heap,theapx,pages_stat_update_count, alloc_count + free_count);

  // allocation sizes
  const size_t bsize = mi_page_usable_block_size(page);
  const uint64_t allocated = (uint64_t)alloc_count * (uint64_t)bsize;
  const uint64_t freed     = (uint64_t)free_count * (uint64_t)bsize;
  mi_assert_internal(allocated <= INT64_MAX);  // safe to cast to int64_t for stats
  mi_assert_internal(freed <= INT64_MAX);

  // adjust stats
  const size_t bin = _mi_bin(bsize);
  if (alloc_count > 0) {
    mi_theapx_stat_counter_increase(heap, theapx, malloc_count, alloc_count);
    mi_theapx_stat_increase(heap, theapx, allocated, allocated);
    mi_theapx_stat_increase(heap, theapx, malloc_bins[bin], alloc_count);
    if (bin==MI_BIN_HUGE) { mi_theapx_stat_increase(heap,theapx,malloc_huge,allocated); } // separate so we have a count and total
    #if MI_STATS==1
    // use coarse total requested bytes (the usable block size, which already excludes the padding)
    mi_theapx_stat_counter_increase(heap, theapx, malloc_requested, allocated);
    #endif
    // deprecated
    if (bin!=MI_BIN_HUGE) {
      mi_theapx_stat_counter_increase(heap, theapx, malloc_normal_count, alloc_count);
      mi_theapx_stat_increase(heap, theapx, malloc_normal, allocated);
    }
    else {
      mi_theapx_stat_counter_increase(heap, theapx, malloc_huge_count, alloc_count);
    }
  }
  if (free_count > 0) {
    mi_theapx_stat_counter_increase(heap, theapx, free_count, free_count);
    mi_theapx_stat_counter_increase(heap, theapx, freed, freed);
    mi_theapx_stat_decrease(heap, theapx, allocated, freed);
    mi_theapx_stat_decrease(heap, theapx, malloc_bins[bin], free_count);
    if (bin==MI_BIN_HUGE) { mi_theapx_stat_decrease(heap, theapx, malloc_huge, freed); }
    // deprecated
    if (bin!=MI_BIN_HUGE) { mi_theapx_stat_decrease(heap, theapx, malloc_normal, freed); }
  }

  // merge the theap stats into the heap once N bytes were allocated or freed since the last merge
  if (theapx != NULL) {
    const size_t threshold = mi_option_get_size(mi_option_stats_merge_threshold);
    const int64_t churn = theapx->stats.allocated.total + theapx->stats.freed.total;
    if (threshold > 0 && churn >= (int64_t)threshold) {
      _mi_theap_merge_stats(theapx);
    }
  }
}

// Update stats for a page (and sample countdown). Can be called with a NULL `theapx`
static void mi_theap_page_update_stats(mi_theap_t* theapx, mi_page_t* page) {
  mi_assert_internal(mi_page_alloc_count(page) + mi_page_last_used(page) >= mi_page_used(page));
  mi_assert_internal(mi_page_alloc_count(page) >= mi_page_last_alloc(page));

  // get stat counts
  const size_t used = mi_page_used(page);
  const size_t alloc_count = mi_page_alloc_count(page);
  const size_t last_used = mi_page_last_used(page);
  mi_assert_internal(last_used + alloc_count >= used);
  mi_assert_internal(used <= UINT16_MAX);
  const size_t free_count = last_used + alloc_count - used;

  #if MI_SAMPLE==1  // for ==2 it is already counted in every `alloc/alloc.c:mi_page_alloc_zero`
  if (theapx!=NULL) { mi_theap_adjust_sample_countdown(theapx,page,alloc_count); }
  #endif

  // update stats?
  if (alloc_count + free_count == 0) {
    mi_assert_internal(last_used == used);
    return;
  }

  mi_page_reset_alloc_count(page);
  mi_theap_page_merge_stats(theapx, page, alloc_count, free_count);
}

// Update stats for a page (and sample countdown)
void _mi_page_update_stats(mi_page_t* page) {         // called on abandoned pages etc.
  mi_theap_page_update_stats(mi_theap_of_page(page),page);
}

#else
// Without statistics we still reset the `alloc_count` so it cannot overflow (see `_mi_page_update_sample_countdown`)
void _mi_page_update_stats(mi_page_t* page) {
  #if MI_SAMPLE==1
  mi_theap_t* const theap = mi_theap_of_page(page);
  if (theap!=NULL) { mi_theap_adjust_sample_countdown(theap,page,mi_page_alloc_count(page)); }
  #endif
  mi_page_reset_alloc_count(page);
}
#endif

// Update the sampling countdown based on the current allocation count of the page
void _mi_page_update_sample_countdown(mi_page_t* page)
{
  mi_assert_internal(mi_page_is_owned(page));
  // note: the thread_id may not match and even be abandoned if we free multi-threaded (so we can not always use page->theap)

  // adjust count down
  const size_t alloc_count = mi_page_alloc_count(page);
  if (alloc_count==0) {
    return;
  }
  else if mi_unlikely(alloc_count>=0x8000) {  // if the count could overflow, update stats so the alloc_count is reset
    _mi_page_update_stats(page);              // calls mi_theap_adjust_sample_countdown if needed
  }
  #if MI_SAMPLE==1  // if ==2 the countdown is already always counted in `alloc/alloc.c:mi_page_alloc_zero_ex`
  else {
    mi_theap_t* theap = mi_theap_of_page(page); // careful as the theap might not be valid
    if (theap==NULL) return;                    // can be NULL on heap_destroy or metadata allocation etc. (and it is safe to ignore the sampling for now)
    mi_theap_adjust_sample_countdown(theap,page,alloc_count);
    // update last_alloc to the current alloc_count
    mi_assert_internal(alloc_count <= UINT16_MAX);
    #if MI_SIZE_SIZE >= 8
      page->xused.used_alloc = (alloc_count << 48) | (page->xused.used_alloc & (~MI_ZU(0) >> 16));
    #else
      page->xlast_alloc = (uint16_t)alloc_count;
    #endif
    mi_assert_internal(mi_page_alloc_count(page) + mi_page_last_used(page) >= mi_page_used(page));
    mi_assert_internal(mi_page_alloc_count(page) >= mi_page_last_alloc(page));
  }
  #endif
}


/* -----------------------------------------------------------
  Page collect the `local_free` and `thread_free` lists
----------------------------------------------------------- */

static void mi_page_thread_collect_to_local(mi_page_t* page, mi_block_t* head)
{
  if (head == NULL) return;

  // find the last block in the list -- also to get a proper use count (without data races)
  // and check each link to detect use-after-free writes (with the bounds hoisted out of the loop)
  const size_t max_count = page->capacity; // cannot collect more than capacity
  #if !MI_ENCODE_FREELIST
  const size_t capacity_size = mi_page_capacity_size(page);
  #endif
  size_t count = 0;
  mi_block_t* last = NULL;
  mi_block_t* next = head;
  do {
    #if !MI_ENCODE_FREELIST  // (otherwise links are already checked in `mi_block_next`)
    if mi_unlikely(!mi_block_next_is_valid(page, next, capacity_size)) {
      _mi_error_block_next_is_invalid(page, last, next);
      return; // the thread-free items cannot be freed
    }
    #endif
    count++;
    last = next;
    next = mi_block_next(page, last);
  } while (next != NULL && count <= max_count);

  // if `count > max_count` there was a memory corruption (possibly infinite list due to double multi-threaded free)
  if mi_unlikely(count > max_count) {
    _mi_error_message(EFAULT, "corrupted thread-free list in page %p with block size %zu (possibly due to a cross-thread double free)\n", page, mi_page_block_size(page));
    return; // the thread-free items cannot be freed
  }
  // if `count > page->used` there was another kind memory corruption (either in the page meta-data or in the linked list)
  else if mi_unlikely(count > mi_page_used(page)) {
    _mi_error_message(EFAULT, "corrupted meta-data in thread-free list in page %p with block size %zu\n", page, mi_page_block_size(page));
    return; // the thread-free items cannot be freed
  }

  // and append the current local free list
  mi_block_set_next(page, last, page->local_free);
  page->local_free = head;

  // update counts now
  mi_assert_internal(count <= UINT16_MAX);
  mi_assert_internal(mi_page_used(page) >= count);
  page->xused.used_alloc -= count; // page->used = page->used - (uint16_t)count;
  if (mi_page_all_free(page)) { _mi_page_qsbr_clear(page); }
}

// Collect the thread free list while preserving ownership.
// Returns `true` if a non-empty thread free list was collected.
static bool mi_page_thread_free_collect(mi_page_t* page)
{
  if mi_likely(mi_page_thread_free(page) == NULL) return false;
  const mi_thread_free_t tfree = mi_atomic_and_acquire(&page->xthread_free, mi_tf_create(NULL, true));
  mi_block_t* const head = mi_tf_block(tfree);
  if (head == NULL) return false;

  // and move it to the local list
  mi_page_thread_collect_to_local(page, head);
  return true;
}


#if (MI_SECURE_HOLD_BACK > 0)
// Keep a random number (1 to `MI_SECURE_HOLD_BACK`) of the most recently freed blocks in the local free list
// and return the rest; if the list is too short, all blocks are returned.
static mi_block_t* mi_page_local_free_hold_back(mi_page_t* page) {
  mi_block_t* const head = page->local_free;
  #if (MI_ENCODE_FREELIST || MI_PADDING)
  const size_t rnd = mi_random_shuffle(page->keys[1] ^ mi_addr(head) ^ mi_page_used(page));
  #else
  const size_t rnd = mi_random_shuffle(mi_addr(page) ^ mi_addr(head) ^ mi_page_used(page));
  #endif
  size_t n = 1 + (rnd % MI_SECURE_HOLD_BACK);
  mi_block_t* last = head;
  mi_block_t* next = mi_block_next(page, last);
  while (next != NULL && --n > 0) {
    last = next;
    next = mi_block_next(page, last);
  }
  if (next == NULL) {
    page->local_free = NULL;
    return head;
  }
  mi_block_set_next(page, last, NULL);
  return next;
}
#endif

// Move the local free list to the (empty) free list regardless of any reuse delay.
static inline void mi_page_local_free_to_free_now(mi_page_t* page) {
  mi_assert_internal(page->free == NULL && page->local_free != NULL);
  #if (MI_SECURE_HOLD_BACK > 0)
  page->free = mi_page_local_free_hold_back(page);
  #else
  page->free = page->local_free;
  page->local_free = NULL;
  #endif
  page->free_is_zero = mi_page_is_zero_on_free(page);
}

#if (MI_SECURE_REUSE_DELAY > 0) || (MI_SECURE_HOLD_BACK > 0)
// Move the local free list to the (empty) free list; returns `false` if reuse is delayed (see `MI_SECURE_REUSE_DELAY`).
static inline bool mi_page_local_free_to_free(mi_page_t* page) {
  if (mi_page_reuse_is_delayed(page)) return false;
  mi_page_local_free_to_free_now(page);
  return true;
}

void _mi_page_local_free_to_free_now(mi_page_t* page) {
  mi_page_local_free_to_free_now(page);
}

bool _mi_page_local_free_to_free(mi_page_t* page) {
  return mi_page_local_free_to_free(page);
}
#endif

bool _mi_page_free_collect(mi_page_t* page, bool force) {
  mi_assert_internal(page!=NULL);

  // collect the thread free list
  const bool collected_xfree = mi_page_thread_free_collect(page);

  // and the local free list
  if (page->local_free != NULL) {
    if mi_likely(page->free == NULL) {
      // usual case
      #if (MI_SECURE_REUSE_DELAY > 0)
      // blocks freed by other threads are not delayed (as they were already delayed in the thread-free list)
      if (collected_xfree) { mi_page_local_free_to_free_now(page); }
                      else { mi_page_local_free_to_free(page); }
      #else
      mi_page_local_free_to_free_now(page);
      #endif
    }
    if (force && page->local_free != NULL) {
      // append -- only on shutdown (force) as this is a linear operation
      const size_t max_count = page->capacity;
      #if !MI_ENCODE_FREELIST
      const size_t capacity_size = mi_page_capacity_size(page);
      #endif
      size_t count = 0;
      mi_block_t* tail = NULL;
      mi_block_t* next = page->local_free;
      do {
        #if !MI_ENCODE_FREELIST  // (otherwise links are already checked in `mi_block_next`)
        if mi_unlikely(!mi_block_next_is_valid(page, next, capacity_size)) {
          _mi_error_block_next_is_invalid(page, tail, next);
          break;  // truncate the list at `tail` (or drop it if the head is invalid)
        }
        #endif
        count++;
        tail = next;
        next = mi_block_next(page, tail);
      } while (next != NULL && count <= max_count);

      // if `count > max_count` there was a memory corruption (possibly infinite list due to a double free)
      if mi_unlikely(count > max_count) {
        _mi_error_message(EFAULT, "corrupted local-free list in page %p with block size %zu (possibly due to a double free)\n", page, mi_page_block_size(page));
      }
      if mi_likely(tail != NULL) {
        mi_block_set_next(page, tail, page->free);
        page->free = page->local_free;
        page->free_is_zero = false;
      }
      page->local_free = NULL;
    }
    _mi_page_update_sample_countdown(page);
  }
  mi_assert_internal(!force || page->local_free == NULL);
  return collected_xfree;
}

// Collect elements in the thread-free list starting at `head`. This is an optimized
// version of `_mi_page_free_collect` to be used from `alloc/free.c:_mi_free_collect_mt` that avoids atomic access to `xthread_free`.
// returns a possibly updated expected value for the thread_free pointer.
//
// `head` must be in the `xthread_free` list. It will not collect `head` itself
// so the `used` count is not fully updated in general. However, if the `head` is
// the last remaining element, it will be collected and the used count will become `0` (so `mi_page_all_free` becomes true).
mi_block_t* _mi_page_free_collect_partly(mi_page_t* page, mi_block_t* head) {
  mi_assert_internal(mi_page_is_owned(page));
  if (head == NULL) return NULL;
  mi_block_t* next = mi_block_next(page,head);  // we cannot collect the head element itself as `page->thread_free` may point to it (and we want to avoid atomic ops)
  if (next != NULL) {
    mi_block_set_next(page, head, NULL);
    mi_page_thread_collect_to_local(page, next);
    if (page->local_free != NULL && page->free == NULL) {
      mi_page_local_free_to_free_now(page);  // not delayed as these were freed by other threads
      _mi_page_update_sample_countdown(page);
    }
  }
  if (mi_page_used(page) == 1) {
    // all elements are free'd since we skipped the `head` element itself
    mi_assert_internal(mi_tf_block(mi_atomic_load_relaxed(&page->xthread_free)) == head);
    mi_assert_internal(mi_block_next(page,head) == NULL);
    _mi_page_free_collect(page, false);  // collect the final element
    return NULL;
  }
  else {
    return head;
  }
}


/* -----------------------------------------------------------
  Page fresh and retire
----------------------------------------------------------- */

// called from `mi_free` on a reclaim, and fresh_alloc if we get an abandoned page
bool _mi_theap_page_reclaim(mi_theap_t* theap, mi_page_t* page)
{
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_assert_internal(mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(!mi_page_is_abandoned_mapped(page));  // already removed from the abandoned map
  mi_assert_internal(mi_theap_is_initialized(theap));

  mi_page_set_theap(page,theap);
  _mi_page_free_collect(page, false); // ensure used count is up to date

  mi_page_queue_t* pq = mi_theap_page_queue_of(theap, page);
  mi_page_queue_push_at_end(theap, pq, page);
  if (!mi_page_all_free(page)) { _mi_page_qsbr_clear(page); }
  mi_assert_expensive(_mi_page_is_valid(page));
  return (mi_page_all_free(page) && _mi_page_free(page, pq));
}

// Abandon a page. If `owner_done` is set, the owning theap abandons all its pages (e.g. as its thread terminates)
// and `mi_thread_id_of` will return 0 for blocks in this page (until another thread reclaims it).
void _mi_page_abandon(mi_page_t* page, mi_page_queue_t* pq, bool owner_done) {
  _mi_page_free_collect(page, false); // ensure used count is up to date
  pq = mi_page_queue_of(page); // a preceding release attempt may have unfulled the page
  if (mi_page_all_free(page) && _mi_page_free(page, pq)) return;
  pq = mi_page_queue_of(page); // _mi_page_free may have moved an empty page out of the full queue
  mi_page_queue_remove(pq, page);
  mi_theap_t* theap = page->theap;
  mi_page_qsbr_unlink(page);
  #if MI_QSBR
  if (owner_done) { mi_page_qsbr_set_thread_id(page, 0); }  // while we still own the page
  #else
  MI_UNUSED(owner_done);
  #endif
  mi_page_set_theap(page, NULL);
  page->theap = theap; // don't actually set theap to NULL so we can reclaim_on_free within the same theap
  _mi_arenas_page_abandon(page, theap);
}


// allocate a fresh page from an arena
mi_page_t* _mi_page_fresh_alloc(mi_theap_t* theap, mi_page_queue_t* pq, size_t block_size, size_t page_alignment) {
  #if !MI_HUGE_PAGE_ABANDON
  mi_assert_internal(pq != NULL);
  mi_assert_internal(mi_theap_contains_queue(theap, pq));
  mi_assert_internal(page_alignment > 0 || block_size > MI_LARGE_MAX_OBJ_SIZE || block_size == pq->block_size);
  #endif
  mi_page_t* page;
  bool reclaimed;
  do {
    page = _mi_arenas_page_alloc(theap, block_size, page_alignment);
    if (page == NULL) {
      // out-of-memory
      return NULL;
    }
    reclaimed = mi_page_is_abandoned(page);
  } while (reclaimed && _mi_theap_page_reclaim(theap, page));
  if (reclaimed) {
    #if (MI_SECURE_REUSE_DELAY > 0)
    if (!mi_page_immediate_available(page) && page->local_free != NULL) { _mi_page_free_collect(page, true); }
    #endif
    if (!mi_page_immediate_available(page)) {
      if (mi_page_is_expandable(page)) {
        if (!_mi_page_extend_free(theap, page)) {
          // cannot commit
          _mi_page_abandon(page,pq,false);
          return NULL;
        };
      }
      else {
        mi_assert(false); // should not happen?
        return NULL;
      }
    }
  }
  else if (pq != NULL) {
    mi_page_queue_push(theap, pq, page);
  }
  mi_assert_internal(pq!=NULL || mi_page_block_size(page) >= block_size);
  mi_assert_expensive(_mi_page_is_valid(page));
  return page;
}

// Get a fresh page to use
mi_page_t* _mi_page_fresh(mi_theap_t* theap, mi_page_queue_t* pq) {
  mi_assert_internal(mi_theap_contains_queue(theap, pq));
  mi_page_t* page = _mi_page_fresh_alloc(theap, pq, pq->block_size, 0);
  if (page==NULL) return NULL;
  mi_assert_internal(pq->block_size==mi_page_block_size(page));
  mi_assert_internal(pq==mi_theap_page_queue_of(theap, page));
  return page;
}


/* -----------------------------------------------------------
  Unfull, abandon, free and retire
----------------------------------------------------------- */

// Move a page from the full list back to a regular list (called from thread-local mi_free)
void _mi_page_unfull(mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(mi_page_is_in_full(page));
  mi_assert_internal(!mi_page_theap(page)->allow_page_abandon);
  if (!mi_page_is_in_full(page)) return;

  mi_theap_t* theap = mi_page_theap(page);
  mi_page_queue_t* pqfull = &theap->pages[MI_BIN_FULL];
  mi_page_set_in_full(page, false); // to get the right queue
  mi_page_queue_t* pq = mi_theap_page_queue_of(theap, page);
  mi_page_set_in_full(page, true);
  mi_page_queue_enqueue_from_full(pq, pqfull, page);
}

void _mi_page_to_full(mi_page_t* page, mi_page_queue_t* pq) {
  mi_assert_internal(pq == mi_page_queue_of(page));
  mi_assert_internal(!mi_page_immediate_available(page));
  mi_assert_internal(!mi_page_is_in_full(page));

  mi_theap_t* theap = mi_page_theap(page);
  if (theap->allow_page_abandon && !(mi_heap_qsbr_is_enabled(theap->heap) && mi_page_queue_is_huge(pq))) {
    // abandon full pages (this is the usual case in order to allow for sharing of memory between theaps)
    _mi_page_abandon(page, pq, false);
  }
  else if (mi_page_queue_is_huge(pq)) {
    // keep huge pages with their owner for QSBR or when abandonment is disabled;
    // the huge queue is never searched for free blocks and is collected before each huge allocation
    _mi_page_update_stats(page);
  }
  else if (!mi_page_is_in_full(page)) {
    // put full pages in a theap local queue (this is for theaps that cannot abandon, for example, if the theap can be destroyed)
    mi_page_queue_enqueue_from(&mi_page_theap(page)->pages[MI_BIN_FULL], pq, page);
    _mi_page_free_collect(page, false);  // try to collect right away in case another thread freed just before MI_USE_DELAYED_FREE was set
    _mi_page_update_stats(page);         // we must update stats here as mi_theap_collect does not normally visit full pages
  }
}


// Before an empty page is freed, check that its free lists hold exactly `capacity` blocks. A double free
// lowers the used count, so a page could otherwise be freed (and reused) while one of its blocks is still in use.
// (a cycle makes the lists too long, and a block that was allocated again has a cleared link which fails to decode)
void _mi_page_check_free_lists(const mi_page_t* page) {
  #if MI_SECURE>=1
  const size_t capacity = page->capacity;
  size_t count = 0;
  for (const mi_block_t* b = page->free; b != NULL && count <= capacity; b = mi_block_next(page, b)) { count++; }
  for (const mi_block_t* b = page->local_free; b != NULL && count <= capacity; b = mi_block_next(page, b)) { count++; }
  if mi_unlikely(count != capacity) {
    _mi_error_message(EFAULT, "corrupted free list in an empty page %p with block size %zu (possibly due to a double free)\n", page, mi_page_block_size(page));
  }
  #else
  MI_UNUSED(page);
  #endif
}

// Physically release an empty page after its grace period has finished.
static void mi_page_free_direct(mi_page_t* page, mi_page_queue_t* pq) {
  mi_assert_internal(page != NULL);
  mi_assert_internal(_mi_page_qsbr_is_clear(page));
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(pq == mi_page_queue_of(page));
  mi_assert_internal(mi_page_all_free(page));
  // mi_assert_internal(mi_page_thread_free_flag(page)!=MI_DELAYED_FREEING);

  // remove from the page list
  // (no need to do _mi_theap_delayed_free first as all blocks are already free)
  mi_page_queue_remove(pq, page);
  _mi_page_check_free_lists(page);

  // and free it
  mi_theap_t* theap = mi_page_theap(page); mi_assert_internal(theap!=NULL);
  mi_page_set_theap(page,NULL);
  _mi_arenas_page_free_direct(page, theap);
  // _mi_arenas_collect(false, false, theap->tld);  // allow purging
}

// Release an empty page (or retain it until its grace period has finished).
bool _mi_page_free(mi_page_t* page, mi_page_queue_t* pq) {
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(pq == mi_page_queue_of(page));

  // no more aligned blocks in here
  mi_page_set_has_interior_pointers(page, false);

  // try to free
  if mi_unlikely(_mi_page_qsbr_should_delay(page)) {
    // for pages that are delayed due to qsbr (e.g. in CPython)
    page->retire_expire = 0;
    if (mi_page_is_in_full(page)) { _mi_page_unfull(page); }
    mi_page_qsbr_enqueue(page, mi_page_theap(page));
    return false;
  }
  else {
    // free the page
    _mi_page_qsbr_clear(page);
    mi_page_free_direct(page, pq);
    return true;
  }
}

#ifndef MI_RETIRE_CYCLES
#define MI_RETIRE_CYCLES      (32)      /* keep a retired page around for about N "admin cycles" before free'ing it */
#endif
#ifndef MI_RETIRE_MAX_PAGES
#define MI_RETIRE_MAX_PAGES   (3)       /* keep at most N pages per size bin as retired */
#endif

// Retire a page with no more used blocks
// Important to not retire too quickly though as new
// allocations might coming.
//
// Note: called from `mi_free` and benchmarks often
// trigger this due to freeing everything and then
// allocating again so careful when changing this.
void _mi_page_retire(mi_page_t* page) mi_attr_noexcept {
  mi_assert_internal(page != NULL);
  mi_assert_expensive(_mi_page_is_valid(page));
  mi_assert_internal(mi_page_all_free(page));

  if (page->retire_expire!=0) return;  // already retired, just keep it retired
  mi_page_set_has_interior_pointers(page, false);

  // don't retire too often..
  // (or we end up retiring and re-allocating most of the time)
  // NOTE: refine this more: we should not retire if this
  // is the only page left with free blocks. It is not clear
  // how to check this efficiently though...
  // for now, we don't retire if it is the only page left of this size class.
  mi_page_queue_t* pq = mi_page_queue_of(page);
  #if MI_RETIRE_CYCLES > 0
  const size_t bsize = mi_page_block_size(page);
  if mi_likely( pq->count <= MI_RETIRE_MAX_PAGES && !mi_page_queue_is_special(pq)) {  // not full or huge queue?
    // memory tagged pages retire up to larger sizes to reuse them with their current tags (instead of fresh random ones)
    if (pq->count==1 || bsize < MI_SMALL_SIZE_MAX || (mi_page_is_memtag(page) && bsize <= MI_SMALL_MAX_OBJ_SIZE)) {
      mi_theap_t* theap = mi_page_theap(page);
      mi_theap_stat_counter_increase(theap, pages_retire, 1);
      page->retire_expire = (bsize <= MI_SMALL_MAX_OBJ_SIZE || mi_page_is_memtag(page) ? MI_RETIRE_CYCLES : MI_RETIRE_CYCLES/4);
      mi_assert_internal(pq >= theap->pages);
      const size_t index = pq - theap->pages;
      mi_assert_internal(index < MI_BIN_FULL && index < MI_BIN_HUGE);
      if (index < theap->page_retired_min) theap->page_retired_min = index;
      if (index > theap->page_retired_max) theap->page_retired_max = index;
      mi_assert_internal(mi_page_all_free(page));
      return; // don't free after all
    }
  }
  #endif
  _mi_page_free(page, pq);
}


#define MI_FULL_COLLECT_MAX_PAGES  (8)   /* visit at most N full (or huge) pages per collect (unless forced) */

// Collect pages in the full or huge queue that had blocks freed by other threads.
// Visits at most MI_FULL_COLLECT_MAX_PAGES pages (unless forced) and rotates the queue so the next call continues after them.
// Each freed or revived page extends the budget by MI_FULL_COLLECT_MAX_PAGES so we keep scanning while cross-thread
// frees revive pages (but visit each page at most once per call). Full pages are only revived once they are no longer
// mostly used, which avoids churning pages with just a few free blocks between the queues.
// note: normally full and huge pages get immediately abandoned and these queues stay (almost) empty;
// this is used if abandoning is disabled for the theap, or for QSBR-protected huge pages.
static void mi_theap_collect_special_pages(mi_page_queue_t* pq, bool force) {
  mi_assert_internal(mi_page_queue_is_special(pq));
  mi_page_t* page = pq->first;
  size_t budget = MI_FULL_COLLECT_MAX_PAGES;
  for (size_t count = 0; page != NULL && (force || count < budget); count++) {
    mi_page_t* next = page->next;         // get next in case we free the page
    _mi_page_free_collect(page, false);   // register concurrent free's
    if (mi_page_all_free(page)) {
      budget += MI_FULL_COLLECT_MAX_PAGES;
      _mi_page_free(page, pq);
    }
    else if (!mi_page_is_mostly_used(page)) {
      mi_assert_internal(mi_page_queue_is_full(pq));  // huge pages contain a single block
      budget += MI_FULL_COLLECT_MAX_PAGES;
      _mi_page_unfull(page);
    }
    page = next;
  }
  if (page != NULL) { mi_page_queue_rotate_to(pq, page); }
}

// Before allocating a fresh huge page, free huge pages whose block was freed by another thread.
void _mi_theap_collect_huge_pages(mi_theap_t* theap) {
  _mi_heap_qsbr_collect_delayed(theap->heap);
  if (theap->allow_page_abandon && !mi_heap_qsbr_is_enabled(theap->heap)) return;  // unprotected huge pages were abandoned
  mi_theap_collect_special_pages(&theap->pages[MI_BIN_HUGE], false);
}

static void mi_page_try_retire(mi_page_queue_t* pq, mi_page_t* page, size_t bin, bool force, size_t* min, size_t* max) {
  mi_assert_internal(page!=NULL && page->retire_expire!=0);
  if (mi_page_all_free(page)) {
    page->retire_expire--;
    if (page->retire_expire == 0 || force) {
      _mi_page_free(page, pq);
    }
    else {
      // keep retired, update min/max
      if (bin < *min) *min = bin;
      if (bin > *max) *max = bin;
    }
  }
  else {
    page->retire_expire = 0;
  }
}

// free retired pages: we don't need to look at the entire queues
// since we only retire pages in queues of at most MI_RETIRE_MAX_PAGES pages.
void _mi_theap_collect_retired(mi_theap_t* theap, bool force) {
  size_t min = MI_BIN_FULL;
  size_t max = 0;
  for(size_t bin = theap->page_retired_min; bin <= theap->page_retired_max; bin++) {
    mi_page_queue_t* pq  = &theap->pages[bin];
    mi_page_t* page      = pq->first;
    for (int i = 0; i<MI_RETIRE_MAX_PAGES && page!=NULL; i++) {
      mi_page_t* next = page->next;
      if (page->retire_expire!=0) { mi_page_try_retire(pq,page,bin,force,&min,&max); }
      page = next;
    }
  }
  theap->page_retired_min = min;
  theap->page_retired_max = max;
  if (!theap->allow_page_abandon) {
    mi_theap_collect_special_pages(&theap->pages[MI_BIN_FULL], force);
  }
  _mi_theap_qsbr_collect_delayed(theap);
}




/* -----------------------------------------------------------
  Initialize the initial free list in a page.
  In secure mode we initialize a randomized list by
  alternating between slices.
----------------------------------------------------------- */

// The number of slices bounds the randomness of the next allocated block. Fewer slices are much
// faster as the hardware prefetcher can still follow the interleaved sequential runs (and 4 slices
// already make the next block unpredictable). At higher secure levels we use up to 64 slices.
#ifndef MI_MAX_SLICE_SHIFT
#if MI_SECURE>=4
#define MI_MAX_SLICE_SHIFT  (6)   // at most 64 slices
#else
#define MI_MAX_SLICE_SHIFT  (2)   // at most 4 slices
#endif
#endif
#define MI_MAX_SLICES       (1UL << MI_MAX_SLICE_SHIFT)
#define MI_MIN_SLICES       (2)

static void mi_page_free_list_extend_secure(mi_theap_t* const theap, mi_page_t* const page, const size_t bsize, const size_t extend, const bool memtag) {
  #if !MI_SECURE_RANDOM_EXTEND
  mi_assert_internal(page->free == NULL);
  #if !(MI_SECURE_REUSE_DELAY || MI_SECURE_HOLD_BACK)
  mi_assert_internal(page->local_free == NULL);
  #endif
  #endif
  mi_assert_internal(page->capacity + extend <= page->reserved);
  mi_assert_internal(bsize == mi_page_block_size(page));

  // initialize a randomized free list
  // set up `slice_count` slices to alternate between
  size_t shift = MI_MAX_SLICE_SHIFT;
  while ((extend >> shift) == 0) {
    shift--;
  }
  const size_t slice_count = (size_t)1U << shift;
  const size_t slice_extend = extend / slice_count;
  mi_assert_internal(slice_extend >= 1);
  mi_block_t* blocks[MI_MAX_SLICES];   // current start of the slice
  size_t      counts[MI_MAX_SLICES];   // available objects in the slice
  for (size_t i = 0; i < slice_count; i++) {
    blocks[i] = mi_page_block_at(page, bsize, page->capacity + i*slice_extend, memtag);
    counts[i] = slice_extend;
  }
  counts[slice_count-1] += (extend % slice_count);  // final slice holds the modulus too (todo: distribute evenly?)

  // and initialize the free list by randomly threading through them
  // set up first element
  const size_t r = _mi_theap_random_next(theap);
  size_t current = r % slice_count;
  counts[current]--;
  mi_block_t* const free_start = blocks[current];
  // and iterate through the rest; use `random_shuffle` for performance
  size_t rnd = mi_random_shuffle(r|1); // ensure not 0
  for (size_t i = 1; i < extend; i++) {
    // call random_shuffle only every SIZE_SIZE rounds
    const size_t round = i%MI_SIZE_SIZE;
    if (round == 0) rnd = mi_random_shuffle(rnd);
    // select a random next slice index
    size_t next = ((rnd >> 8*round) & (slice_count-1));
    while (counts[next]==0) {                            // ensure it still has space
      next++;
      if (next==slice_count) next = 0;
    }
    // and link the current block to it
    counts[next]--;
    mi_block_t* const block = blocks[current];
    blocks[current] = mi_block_after(block, bsize, memtag && (counts[current] > 0 || current == next));  // bump to the following block
    mi_block_set_next(page, block, blocks[next]);   // and set next; note: we may have `current == next`
    current = next;
  }
  // prepend to the free list (usually NULL)
  mi_block_set_next(page, blocks[current], page->free);  // end of the list
  page->free = free_start;
}

static mi_decl_noinline void mi_page_free_list_extend( mi_page_t* const page, const size_t bsize, const size_t extend, const bool memtag)
{
  #if !MI_SECURE_RANDOM_EXTEND
  mi_assert_internal(page->free == NULL);
  #if !(MI_SECURE_REUSE_DELAY || MI_SECURE_HOLD_BACK)
  mi_assert_internal(page->local_free == NULL);
  #endif
  #endif
  mi_assert_internal(page->capacity + extend <= page->reserved);
  mi_assert_internal(bsize == mi_page_block_size(page));

  mi_block_t* const start = mi_page_block_at(page, bsize, page->capacity, memtag);

  // initialize a sequential free list
  mi_block_t* const last = mi_page_block_at(page, bsize, page->capacity + extend - 1, false);
  mi_block_t* block = start;
  while (mi_addr(block) < mi_addr(last)) {
    mi_block_t* const next = mi_block_after(block, bsize, memtag);
    mi_block_set_next(page,block,next);
    block = next;
  }
  // prepend to free list (usually `NULL`); here `block` is `last` with its memory tag
  mi_block_set_next(page, (memtag ? block : last), page->free);
  page->free = start;
}

/* -----------------------------------------------------------
  Page initialize and extend the capacity
----------------------------------------------------------- */

#define MI_MAX_EXTEND_SIZE    (8*1024)      // heuristic, one or two OS pages seems to work well.
#if MI_SECURE_SHUFFLE_FREELIST
// extend at least by this many (for free list randomization): 32 blocks, but at most `2*MI_MAX_EXTEND_SIZE` bytes (and at least 4 blocks)
#define MI_MIN_EXTEND(bsize)  ((bsize) <= (2*MI_MAX_EXTEND_SIZE)/32 ? 32 : ((bsize) >= (2*MI_MAX_EXTEND_SIZE)/4 ? 4 : (2*MI_MAX_EXTEND_SIZE)/(bsize)))
#else
#define MI_MIN_EXTEND(bsize)  (1)
#endif

// Extend the capacity (up to reserved) by initializing a free list
// We do at most `MI_MAX_EXTEND` to avoid touching too much memory
// Note: we also experimented with "bump" allocation on the first
// allocations but this did not speed up any benchmark (due to an
// extra test in malloc? or cache effects?)
bool _mi_page_extend_free(mi_theap_t* theap, mi_page_t* page) {
  mi_assert_expensive(mi_page_is_valid_init(page));
  #if !MI_SECURE_RANDOM_EXTEND
  mi_assert(page->free == NULL);
  #if !(MI_SECURE_REUSE_DELAY || MI_SECURE_HOLD_BACK)
  mi_assert(page->local_free == NULL);
  #endif
  if (page->free != NULL) return true;
  #endif
  if (page->capacity >= page->reserved) return true;

  size_t page_size;
  //uint8_t* page_start =
  mi_page_area(page, &page_size);
  mi_theap_stat_counter_increase(theap, pages_extended, 1);

  // calculate the extend count
  const size_t bsize = mi_page_block_size(page);
  size_t extend = (size_t)page->reserved - page->capacity;
  mi_assert_internal(extend > 0);

  const size_t min_extend = MI_MIN_EXTEND(bsize);
  size_t max_extend = min_extend;
  if (bsize < MI_MAX_EXTEND_SIZE) {
    // extend up to an `MI_MAX_EXTEND_SIZE` aligned address (as the page start can be offset) to avoid touching an extra OS page
    const mi_addr_t start = mi_addr(mi_page_block_at(page, bsize, page->capacity, false));
    const mi_addr_t end = mi_align_down(start + MI_MAX_EXTEND_SIZE, MI_MAX_EXTEND_SIZE);
    max_extend = (size_t)(end - start) / bsize;
    if (max_extend < min_extend) { max_extend = min_extend; }
  }
  mi_assert_internal(max_extend > 0);

  if (extend > max_extend) {
    // ensure we don't touch memory beyond the page to reduce page commit.
    // the `lean` benchmark tests this. Going from 1 to 8 increases rss by 50%.
    extend = max_extend;
  }

  mi_assert_internal(extend > 0 && extend + page->capacity <= page->reserved);
  mi_assert_internal(extend < (1UL<<16));

  // commit on demand?
  const size_t slice_committed = mi_page_slice_committed(page);
  if (slice_committed > 0) {
    // reduce extend if it commits more than an arena slice
    if ((extend * bsize) > MI_ARENA_SLICE_SIZE) {
      extend = mi_divide_up(MI_ARENA_SLICE_SIZE, bsize);
    }
    // commit required size
    const size_t needed_size = (page->capacity + extend)*bsize;
    mi_assert_internal(needed_size <= page_size);
    size_t needed_commit = mi_align_up( mi_page_slice_offset_of(page, needed_size), mi_page_min_commit_size());
    if (page->memid.guard_end && needed_commit > mi_page_guard_end_offset(page)) {
      // the previous alignup could extend the commit into the guard page; re-adjust if needed
      needed_commit = mi_page_guard_end_offset(page);
    }
    if (needed_commit > slice_committed) {
      mi_assert_internal(((needed_commit - slice_committed) % _mi_os_page_size()) == 0);
      if (!_mi_os_commit(mi_theap_subproc(theap), mi_page_slice_start(page) + slice_committed, needed_commit - slice_committed, NULL)) {
        return false;
      }
      mi_assert_internal(needed_commit <= UINT16_MAX * _mi_os_page_size());
      page->slice_pcommitted = (uint16_t)(needed_commit / _mi_os_page_size());
    }
  }

  // tag the new blocks
  const bool memtag = mi_page_is_memtag(page);
  if (memtag) {
    mi_memtag_blocks_init(mi_page_start(page) + (page->capacity * bsize), bsize, page->capacity, page->reserved, mi_page_memtag_parity(page), extend);
  }
  // if not initially zero, the free list of a zero-on-free page is no longer zero
  if (!page->memid.initially_zero) { page->free_is_zero = false; }

  // and append the extend the free list
  if (MI_SECURE_SHUFFLE_FREELIST && extend >= MI_MIN_SLICES) {
    mi_page_free_list_extend_secure(theap, page, bsize, extend, memtag);
  }
  else {
    mi_page_free_list_extend(page, bsize, extend, memtag);
  }
  // enable the new free list
  page->capacity += (uint16_t)extend;
  #if MI_FREE_IS_CHECKED
  mi_atomic_store_relaxed(&page->capacity_size, (size_t)page->capacity * bsize);
  #endif
  mi_theap_stat_increase(theap, page_committed, extend * bsize);
  mi_assert_expensive(mi_page_is_valid_init(page));
  return true;
}

// Initialize a fresh page (that is already partially initialized)
mi_decl_nodiscard bool _mi_page_init(mi_theap_t* theap, mi_page_t* page) {
  mi_assert(page != NULL);
  mi_assert(theap!=NULL);
  // page->heap = (mi_is_heap_main(mi_theap_heap(theap)) ? NULL : mi_theap_heap(theap)); // faster for `mi_page_associated_theap`
  // mi_page_set_theap(page, theap);

  size_t page_size;
  uint8_t* page_start = mi_page_area(page, &page_size); MI_UNUSED(page_start);
  mi_track_mem_noaccess(page_start,page_size);
  mi_assert_internal(page_size / mi_page_block_size(page) < (1L<<16));
  mi_assert_internal(page->reserved == 1 || page_size <= MI_LARGE_PAGE_SIZE);  // block offsets fit in a free list link (see `mi_ptr_encode`)
  mi_assert_internal(mi_ptr_memtag(page_start) == 0);  // tagged free list links are relative to an untagged page start (see `mi_ptr_decode`)
  mi_assert_internal(page->reserved > 0);
  #if (MI_PADDING || MI_ENCODE_FREELIST)
  page->keys[0] = _mi_theap_random_next(theap);
  page->keys[1] = _mi_theap_random_next(theap);
  #endif
  #if MI_DEBUG>2
  if (page->memid.initially_zero) {
    mi_track_mem_defined(mi_page_start(page), mi_page_committed(page));
    mi_assert_expensive(mi_mem_is_zero(page_start, mi_page_committed(page)));
  }
  #endif

  mi_assert_internal(page->heap != NULL);
  mi_assert_internal(page->heap == mi_theap_heap(theap));
  mi_assert_internal(page->theap!=NULL);
  mi_assert_internal(page->theap == mi_page_theap(page));
  mi_assert_internal(page->capacity == 0);
  mi_assert_internal(page->free == NULL);
  mi_assert_internal(mi_page_used(page) == 0);
  mi_assert_internal(page->xused.used_alloc == 0);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(page->xthread_free == 1);
  mi_assert_internal(page->next == NULL);
  mi_assert_internal(page->prev == NULL);
  mi_assert_internal(page->retire_expire == 0);
  mi_assert_internal(!mi_page_has_interior_pointers(page));
  #if (MI_PADDING || MI_ENCODE_FREELIST)
  mi_assert_internal(page->keys[0] != 0);
  mi_assert_internal(page->keys[1] != 0);
  #endif
  mi_assert_expensive(mi_page_is_valid_init(page));

  // initialize an initial free list
  if (!_mi_page_extend_free(theap,page)) return false;
  mi_assert(mi_page_immediate_available(page));
  return true;
}
