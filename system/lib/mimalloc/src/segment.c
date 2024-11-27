/* ----------------------------------------------------------------------------
Copyright (c) 2018-2024, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "mimalloc.h"
#include "mimalloc/internal.h"
#include "mimalloc/atomic.h"

#include <string.h>  // memset
#include <stdio.h>

#define MI_PAGE_HUGE_ALIGN  (256*1024)

static uint8_t* mi_segment_raw_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t* page_size);

/* --------------------------------------------------------------------------------
  Segment allocation
  We allocate pages inside bigger "segments" (4MiB on 64-bit). This is to avoid
  splitting VMA's on Linux and reduce fragmentation on other OS's.
  Each thread owns its own segments.

  Currently we have:
  - small pages (64KiB), 64 in one segment
  - medium pages (512KiB), 8 in one segment
  - large pages (4MiB), 1 in one segment
  - huge segments have 1 page in one segment that can be larger than `MI_SEGMENT_SIZE`.
    it is used for blocks `> MI_LARGE_OBJ_SIZE_MAX` or with alignment `> MI_BLOCK_ALIGNMENT_MAX`.

  The memory for a segment is usually committed on demand.
  (i.e. we are careful to not touch the memory until we actually allocate a block there)

  If a  thread ends, it "abandons" pages that still contain live blocks.
  Such segments are abondoned and these can be reclaimed by still running threads,
  (much like work-stealing).
-------------------------------------------------------------------------------- */


/* -----------------------------------------------------------
  Queue of segments containing free pages
----------------------------------------------------------- */

#if (MI_DEBUG>=3)
static bool mi_segment_queue_contains(const mi_segment_queue_t* queue, const mi_segment_t* segment) {
  mi_assert_internal(segment != NULL);
  mi_segment_t* list = queue->first;
  while (list != NULL) {
    if (list == segment) break;
    mi_assert_internal(list->next==NULL || list->next->prev == list);
    mi_assert_internal(list->prev==NULL || list->prev->next == list);
    list = list->next;
  }
  return (list == segment);
}
#endif

/*
static bool mi_segment_queue_is_empty(const mi_segment_queue_t* queue) {
  return (queue->first == NULL);
}
*/

static void mi_segment_queue_remove(mi_segment_queue_t* queue, mi_segment_t* segment) {
  mi_assert_expensive(mi_segment_queue_contains(queue, segment));
  if (segment->prev != NULL) segment->prev->next = segment->next;
  if (segment->next != NULL) segment->next->prev = segment->prev;
  if (segment == queue->first) queue->first = segment->next;
  if (segment == queue->last)  queue->last = segment->prev;
  segment->next = NULL;
  segment->prev = NULL;
}

static void mi_segment_enqueue(mi_segment_queue_t* queue, mi_segment_t* segment) {
  mi_assert_expensive(!mi_segment_queue_contains(queue, segment));
  segment->next = NULL;
  segment->prev = queue->last;
  if (queue->last != NULL) {
    mi_assert_internal(queue->last->next == NULL);
    queue->last->next = segment;
    queue->last = segment;
  }
  else {
    queue->last = queue->first = segment;
  }
}

static mi_segment_queue_t* mi_segment_free_queue_of_kind(mi_page_kind_t kind, mi_segments_tld_t* tld) {
  if (kind == MI_PAGE_SMALL) return &tld->small_free;
  else if (kind == MI_PAGE_MEDIUM) return &tld->medium_free;
  else return NULL;
}

static mi_segment_queue_t* mi_segment_free_queue(const mi_segment_t* segment, mi_segments_tld_t* tld) {
  return mi_segment_free_queue_of_kind(segment->page_kind, tld);
}

// remove from free queue if it is in one
static void mi_segment_remove_from_free_queue(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_segment_queue_t* queue = mi_segment_free_queue(segment, tld); // may be NULL
  bool in_queue = (queue!=NULL && (segment->next != NULL || segment->prev != NULL || queue->first == segment));
  if (in_queue) {
    mi_segment_queue_remove(queue, segment);
  }
}

static void mi_segment_insert_in_free_queue(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_segment_enqueue(mi_segment_free_queue(segment, tld), segment);
}


/* -----------------------------------------------------------
 Invariant checking
----------------------------------------------------------- */

#if (MI_DEBUG >= 2) || (MI_SECURE >= 2)
static size_t mi_segment_page_size(const mi_segment_t* segment) {
  if (segment->capacity > 1) {
    mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM);
    return ((size_t)1 << segment->page_shift);
  }
  else {
    mi_assert_internal(segment->page_kind >= MI_PAGE_LARGE);
    return segment->segment_size;
  }
}
#endif

#if (MI_DEBUG>=2)
static bool mi_pages_purge_contains(const mi_page_t* page, mi_segments_tld_t* tld) {
  mi_page_t* p = tld->pages_purge.first;
  while (p != NULL) {
    if (p == page) return true;
    p = p->next;
  }
  return false;
}
#endif

#if (MI_DEBUG>=3)
static bool mi_segment_is_valid(const mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(segment != NULL);
  mi_assert_internal(_mi_ptr_cookie(segment) == segment->cookie);
  mi_assert_internal(segment->used <= segment->capacity);
  mi_assert_internal(segment->abandoned <= segment->used);
  mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM || segment->capacity == 1); // one large or huge page per segment
  size_t nfree = 0;
  for (size_t i = 0; i < segment->capacity; i++) {
    const mi_page_t* const page = &segment->pages[i];
    if (!page->segment_in_use) {
      nfree++;
    }
    if (page->segment_in_use) {
      mi_assert_expensive(!mi_pages_purge_contains(page, tld));
    }
    mi_assert_internal(page->is_huge == (segment->page_kind == MI_PAGE_HUGE));
  }
  mi_assert_internal(nfree + segment->used == segment->capacity);
  // mi_assert_internal(segment->thread_id == _mi_thread_id() || (segment->thread_id==0)); // or 0
  mi_assert_internal(segment->page_kind == MI_PAGE_HUGE ||
                     (mi_segment_page_size(segment) * segment->capacity == segment->segment_size));
  return true;
}
#endif

static bool mi_page_not_in_queue(const mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert_internal(page != NULL);
  if (page->next != NULL || page->prev != NULL) {
    mi_assert_internal(mi_pages_purge_contains(page, tld));
    return false;
  }
  else {
    // both next and prev are NULL, check for singleton list
    return (tld->pages_purge.first != page && tld->pages_purge.last != page);
  }
}


/* -----------------------------------------------------------
  Guard pages
----------------------------------------------------------- */

static void mi_segment_protect_range(void* p, size_t size, bool protect) {
  if (protect) {
    _mi_os_protect(p, size);
  }
  else {
    _mi_os_unprotect(p, size);
  }
}

static void mi_segment_protect(mi_segment_t* segment, bool protect, mi_os_tld_t* tld) {
  // add/remove guard pages
  if (MI_SECURE != 0) {
    // in secure mode, we set up a protected page in between the segment info and the page data
    const size_t os_psize = _mi_os_page_size();
    mi_assert_internal((segment->segment_info_size - os_psize) >= (sizeof(mi_segment_t) + ((segment->capacity - 1) * sizeof(mi_page_t))));
    mi_assert_internal(((uintptr_t)segment + segment->segment_info_size) % os_psize == 0);
    mi_segment_protect_range((uint8_t*)segment + segment->segment_info_size - os_psize, os_psize, protect);
    #if (MI_SECURE >= 2)
    if (segment->capacity == 1)
    #endif
    {
      // and protect the last (or only) page too
      mi_assert_internal(MI_SECURE <= 1 || segment->page_kind >= MI_PAGE_LARGE);
      uint8_t* start = (uint8_t*)segment + segment->segment_size - os_psize;
      if (protect && !segment->memid.initially_committed) {
        if (protect) {
          // ensure secure page is committed
          if (_mi_os_commit(start, os_psize, NULL, tld->stats)) {  // if this fails that is ok (as it is an unaccessible page)
            mi_segment_protect_range(start, os_psize, protect);
          }
        }
      }
      else {
        mi_segment_protect_range(start, os_psize, protect);
      }
    }
    #if (MI_SECURE >= 2)
    else {
      // or protect every page
      const size_t page_size = mi_segment_page_size(segment);
      for (size_t i = 0; i < segment->capacity; i++) {
        if (segment->pages[i].is_committed) {
          mi_segment_protect_range((uint8_t*)segment + (i+1)*page_size - os_psize, os_psize, protect);
        }
      }
    }
    #endif
  }
}

/* -----------------------------------------------------------
  Page reset
----------------------------------------------------------- */

static void mi_page_purge(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld) {
  // todo: should we purge the guard page as well when MI_SECURE>=2 ?
  mi_assert_internal(page->is_committed);
  mi_assert_internal(!page->segment_in_use);
  if (!segment->allow_purge) return;
  mi_assert_internal(page->used == 0);
  mi_assert_internal(page->free == NULL);
  mi_assert_expensive(!mi_pages_purge_contains(page, tld));
  size_t psize;
  void* start = mi_segment_raw_page_start(segment, page, &psize);
  const bool needs_recommit = _mi_os_purge(start, psize, tld->stats);
  if (needs_recommit) { page->is_committed = false; }
}

static bool mi_page_ensure_committed(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld) {
  if (page->is_committed) return true;
  mi_assert_internal(segment->allow_decommit);
  mi_assert_expensive(!mi_pages_purge_contains(page, tld));

  size_t psize;
  uint8_t* start = mi_segment_raw_page_start(segment, page, &psize);
  bool is_zero = false;
  const size_t gsize = (MI_SECURE >= 2 ? _mi_os_page_size() : 0);
  bool ok = _mi_os_commit(start, psize + gsize, &is_zero, tld->stats);
  if (!ok) return false; // failed to commit!
  page->is_committed = true;
  page->used = 0;
  page->free = NULL;
  page->is_zero_init = is_zero;
  if (gsize > 0) {
    mi_segment_protect_range(start + psize, gsize, true);
  }
  return true;
}


/* -----------------------------------------------------------
  The free page queue
----------------------------------------------------------- */

// we re-use the `free` field for the expiration counter. Since this is a
// a pointer size field while the clock is always 64-bit we need to guard
// against overflow, we use substraction to check for expiry which works
// as long as the reset delay is under (2^30 - 1) milliseconds (~12 days)
static uint32_t mi_page_get_expire( mi_page_t* page ) {
  return (uint32_t)((uintptr_t)page->free);
}

static void mi_page_set_expire( mi_page_t* page, uint32_t expire ) {
  page->free = (mi_block_t*)((uintptr_t)expire);
}

static void mi_page_purge_set_expire(mi_page_t* page) {
  mi_assert_internal(mi_page_get_expire(page)==0);
  uint32_t expire = (uint32_t)_mi_clock_now() + mi_option_get(mi_option_purge_delay);
  mi_page_set_expire(page, expire);
}

// we re-use the `free` field for the expiration counter. Since this is a
// a pointer size field while the clock is always 64-bit we need to guard
// against overflow, we use substraction to check for expiry which work
// as long as the reset delay is under (2^30 - 1) milliseconds (~12 days)
static bool mi_page_purge_is_expired(mi_page_t* page, mi_msecs_t now) {
  int32_t expire = (int32_t)mi_page_get_expire(page);
  return (((int32_t)now - expire) >= 0);
}

static void mi_segment_schedule_purge(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert_internal(!page->segment_in_use);
  mi_assert_internal(mi_page_not_in_queue(page,tld));
  mi_assert_expensive(!mi_pages_purge_contains(page, tld));
  mi_assert_internal(_mi_page_segment(page)==segment);
  if (!segment->allow_purge) return;

  if (mi_option_get(mi_option_purge_delay) == 0) {
    // purge immediately?
    mi_page_purge(segment, page, tld);
  }
  else if (mi_option_get(mi_option_purge_delay) > 0) {   // no purging if the delay is negative
    // otherwise push on the delayed page reset queue
    mi_page_queue_t* pq = &tld->pages_purge;
    // push on top
    mi_page_purge_set_expire(page);
    page->next = pq->first;
    page->prev = NULL;
    if (pq->first == NULL) {
      mi_assert_internal(pq->last == NULL);
      pq->first = pq->last = page;
    }
    else {
      pq->first->prev = page;
      pq->first = page;
    }
  }
}

static void mi_page_purge_remove(mi_page_t* page, mi_segments_tld_t* tld) {
  if (mi_page_not_in_queue(page,tld)) return;

  mi_page_queue_t* pq = &tld->pages_purge;
  mi_assert_internal(pq!=NULL);
  mi_assert_internal(!page->segment_in_use);
  mi_assert_internal(mi_page_get_expire(page) != 0);
  mi_assert_internal(mi_pages_purge_contains(page, tld));
  if (page->prev != NULL) page->prev->next = page->next;
  if (page->next != NULL) page->next->prev = page->prev;
  if (page == pq->last)  pq->last = page->prev;
  if (page == pq->first) pq->first = page->next;
  page->next = page->prev = NULL;
  mi_page_set_expire(page,0);
}

static void mi_segment_remove_all_purges(mi_segment_t* segment, bool force_purge, mi_segments_tld_t* tld) {
  if (segment->memid.is_pinned) return; // never reset in huge OS pages
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (!page->segment_in_use) {
      mi_page_purge_remove(page, tld);
      if (force_purge && page->is_committed) {
        mi_page_purge(segment, page, tld);
      }
    }
    else {
      mi_assert_internal(mi_page_not_in_queue(page,tld));
    }
  }
}

static void mi_pages_try_purge(bool force, mi_segments_tld_t* tld) {
  if (mi_option_get(mi_option_purge_delay) < 0) return;  // purging is not allowed

  mi_msecs_t now = _mi_clock_now();
  mi_page_queue_t* pq = &tld->pages_purge;
  // from oldest up to the first that has not expired yet
  mi_page_t* page = pq->last;
  while (page != NULL && (force || mi_page_purge_is_expired(page,now))) {
    mi_page_t* const prev = page->prev; // save previous field
    mi_page_purge_remove(page, tld);    // remove from the list to maintain invariant for mi_page_purge
    mi_page_purge(_mi_page_segment(page), page, tld);
    page = prev;
  }
  // discard the reset pages from the queue
  pq->last = page;
  if (page != NULL){
    page->next = NULL;
  }
  else {
    pq->first = NULL;
  }
}


/* -----------------------------------------------------------
 Segment size calculations
----------------------------------------------------------- */

static size_t mi_segment_raw_page_size(const mi_segment_t* segment) {
  return (segment->page_kind == MI_PAGE_HUGE ? segment->segment_size : (size_t)1 << segment->page_shift);
}

// Raw start of the page available memory; can be used on uninitialized pages (only `segment_idx` must be set)
// The raw start is not taking aligned block allocation into consideration.
static uint8_t* mi_segment_raw_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t* page_size) {
  size_t   psize = mi_segment_raw_page_size(segment);
  uint8_t* p = (uint8_t*)segment + page->segment_idx * psize;

  if (page->segment_idx == 0) {
    // the first page starts after the segment info (and possible guard page)
    p += segment->segment_info_size;
    psize -= segment->segment_info_size;
  }

#if (MI_SECURE > 1)  // every page has an os guard page
  psize -= _mi_os_page_size();
#elif (MI_SECURE==1) // the last page has an os guard page at the end
  if (page->segment_idx == segment->capacity - 1) {
    psize -= _mi_os_page_size();
  }
#endif

  if (page_size != NULL) *page_size = psize;
  mi_assert_internal(page->block_size == 0 || _mi_ptr_page(p) == page);
  mi_assert_internal(_mi_ptr_segment(p) == segment);
  return p;
}

// Start of the page available memory; can be used on uninitialized pages (only `segment_idx` must be set)
uint8_t* _mi_segment_page_start(const mi_segment_t* segment, const mi_page_t* page, size_t* page_size)
{
  size_t   psize;
  uint8_t* p = mi_segment_raw_page_start(segment, page, &psize);
  const size_t block_size = mi_page_block_size(page);
  if (/*page->segment_idx == 0 &&*/ block_size > 0 && block_size <= MI_MAX_ALIGN_GUARANTEE) {
    // for small and medium objects, ensure the page start is aligned with the block size (PR#66 by kickunderscore)
    mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM);
    size_t adjust = block_size - ((uintptr_t)p % block_size);
    if (adjust < block_size && psize >= block_size + adjust) {
      p += adjust;
      psize -= adjust;
      mi_assert_internal((uintptr_t)p % block_size == 0);
    }
  }
  mi_assert_internal(_mi_is_aligned(p, MI_MAX_ALIGN_SIZE));
  mi_assert_internal(block_size == 0 || block_size > MI_MAX_ALIGN_GUARANTEE || _mi_is_aligned(p,block_size));

  if (page_size != NULL) *page_size = psize;
  mi_assert_internal(_mi_ptr_page(p) == page);
  mi_assert_internal(_mi_ptr_segment(p) == segment);
  return p;
}


static size_t mi_segment_calculate_sizes(size_t capacity, size_t required, size_t* pre_size, size_t* info_size)
{
  const size_t minsize = sizeof(mi_segment_t) + ((capacity - 1) * sizeof(mi_page_t)) + 16 /* padding */;
  size_t guardsize = 0;
  size_t isize     = 0;


  if (MI_SECURE == 0) {
    // normally no guard pages
    #if MI_GUARDED
    isize = _mi_align_up(minsize, _mi_os_page_size());
    #else
    isize = _mi_align_up(minsize, 16 * MI_MAX_ALIGN_SIZE);
    #endif
  }
  else {
    // in secure mode, we set up a protected page in between the segment info
    // and the page data (and one at the end of the segment)
    const size_t page_size = _mi_os_page_size();
    isize = _mi_align_up(minsize, page_size);
    guardsize = page_size;
    //required = _mi_align_up(required, isize + guardsize);
  }

  if (info_size != NULL) *info_size = isize;
  if (pre_size != NULL)  *pre_size  = isize + guardsize;
  return (required==0 ? MI_SEGMENT_SIZE : _mi_align_up( required + isize + 2*guardsize, MI_PAGE_HUGE_ALIGN) );
}


/* ----------------------------------------------------------------------------
Segment caches
We keep a small segment cache per thread to increase local
reuse and avoid setting/clearing guard pages in secure mode.
------------------------------------------------------------------------------- */

static void mi_segments_track_size(long segment_size, mi_segments_tld_t* tld) {
  if (segment_size>=0) _mi_stat_increase(&tld->stats->segments,1);
                  else _mi_stat_decrease(&tld->stats->segments,1);
  tld->count += (segment_size >= 0 ? 1 : -1);
  if (tld->count > tld->peak_count) tld->peak_count = tld->count;
  tld->current_size += segment_size;
  if (tld->current_size > tld->peak_size) tld->peak_size = tld->current_size;
}

static void mi_segment_os_free(mi_segment_t* segment, size_t segment_size, mi_segments_tld_t* tld) {
  segment->thread_id = 0;
  _mi_segment_map_freed_at(segment);
  mi_segments_track_size(-((long)segment_size),tld);
  if (segment->was_reclaimed) {
    tld->reclaim_count--;
    segment->was_reclaimed = false;
  }

  if (MI_SECURE != 0) {
    mi_assert_internal(!segment->memid.is_pinned);
    mi_segment_protect(segment, false, tld->os); // ensure no more guard pages are set
  }

  bool fully_committed = true;
  size_t committed_size = 0;
  const size_t page_size = mi_segment_raw_page_size(segment);
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (page->is_committed)  { committed_size += page_size;  }
    if (!page->is_committed) { fully_committed = false; }
  }
  MI_UNUSED(fully_committed);
  mi_assert_internal((fully_committed && committed_size == segment_size) || (!fully_committed && committed_size < segment_size));

  _mi_arena_free(segment, segment_size, committed_size, segment->memid, tld->stats);
}

// called from `heap_collect`.
void _mi_segments_collect(bool force, mi_segments_tld_t* tld) {
  mi_pages_try_purge(force,tld);
  #if MI_DEBUG>=2
  if (!_mi_is_main_thread()) {
    mi_assert_internal(tld->pages_purge.first == NULL);
    mi_assert_internal(tld->pages_purge.last == NULL);
  }
  #endif
}


/* -----------------------------------------------------------
   Segment allocation
----------------------------------------------------------- */

static mi_segment_t* mi_segment_os_alloc(bool eager_delayed, size_t page_alignment, mi_arena_id_t req_arena_id,
                                         size_t pre_size, size_t info_size, bool commit, size_t segment_size,
                                         mi_segments_tld_t* tld, mi_os_tld_t* tld_os)
{
  mi_memid_t memid;
  bool   allow_large = (!eager_delayed && (MI_SECURE == 0)); // only allow large OS pages once we are no longer lazy
  size_t align_offset = 0;
  size_t alignment = MI_SEGMENT_SIZE;
  if (page_alignment > 0) {
    alignment = page_alignment;
    align_offset = _mi_align_up(pre_size, MI_SEGMENT_SIZE);
    segment_size = segment_size + (align_offset - pre_size);  // adjust the segment size
  }

  mi_segment_t* segment = (mi_segment_t*)_mi_arena_alloc_aligned(segment_size, alignment, align_offset, commit, allow_large, req_arena_id, &memid, tld_os);
  if (segment == NULL) {
    return NULL;  // failed to allocate
  }

  if (!memid.initially_committed) {
    // ensure the initial info is committed
    mi_assert_internal(!memid.is_pinned);
    bool ok = _mi_os_commit(segment, pre_size, NULL, tld_os->stats);
    if (!ok) {
      // commit failed; we cannot touch the memory: free the segment directly and return `NULL`
      _mi_arena_free(segment, segment_size, 0, memid, tld_os->stats);
      return NULL;
    }
  }

  MI_UNUSED(info_size);
  segment->memid = memid;
  segment->allow_decommit = !memid.is_pinned;
  segment->allow_purge = segment->allow_decommit && (mi_option_get(mi_option_purge_delay) >= 0);
  segment->segment_size = segment_size;
  segment->subproc = tld->subproc;
  mi_segments_track_size((long)(segment_size), tld);
  _mi_segment_map_allocated_at(segment);
  return segment;
}

// Allocate a segment from the OS aligned to `MI_SEGMENT_SIZE` .
static mi_segment_t* mi_segment_alloc(size_t required, mi_page_kind_t page_kind, size_t page_shift, size_t page_alignment,
                                      mi_arena_id_t req_arena_id, mi_segments_tld_t* tld, mi_os_tld_t* os_tld)
{
  // required is only > 0 for huge page allocations
  mi_assert_internal((required > 0 && page_kind > MI_PAGE_LARGE)|| (required==0 && page_kind <= MI_PAGE_LARGE));

  // calculate needed sizes first
  size_t capacity;
  if (page_kind == MI_PAGE_HUGE) {
    mi_assert_internal(page_shift == MI_SEGMENT_SHIFT + 1 && required > 0);
    capacity = 1;
  }
  else {
    mi_assert_internal(required == 0 && page_alignment == 0);
    size_t page_size = (size_t)1 << page_shift;
    capacity = MI_SEGMENT_SIZE / page_size;
    mi_assert_internal(MI_SEGMENT_SIZE % page_size == 0);
    mi_assert_internal(capacity >= 1 && capacity <= MI_SMALL_PAGES_PER_SEGMENT);
  }
  size_t info_size;
  size_t pre_size;
  const size_t init_segment_size = mi_segment_calculate_sizes(capacity, required, &pre_size, &info_size);
  mi_assert_internal(init_segment_size >= required);

  // Initialize parameters
  const bool eager_delayed = (page_kind <= MI_PAGE_MEDIUM &&          // don't delay for large objects
                              // !_mi_os_has_overcommit() &&          // never delay on overcommit systems
                              _mi_current_thread_count() > 1 &&       // do not delay for the first N threads
                              tld->peak_count < (size_t)mi_option_get(mi_option_eager_commit_delay));
  const bool eager  = !eager_delayed && mi_option_is_enabled(mi_option_eager_commit);
  const bool init_commit = eager; // || (page_kind >= MI_PAGE_LARGE);

  // Allocate the segment from the OS (segment_size can change due to alignment)
  mi_segment_t* segment = mi_segment_os_alloc(eager_delayed, page_alignment, req_arena_id, pre_size, info_size, init_commit, init_segment_size, tld, os_tld);
  if (segment == NULL) return NULL;
  mi_assert_internal(segment != NULL && (uintptr_t)segment % MI_SEGMENT_SIZE == 0);
  mi_assert_internal(segment->memid.is_pinned ? segment->memid.initially_committed : true);

  // zero the segment info (but not the `mem` fields)
  ptrdiff_t ofs = offsetof(mi_segment_t, next);
  _mi_memzero((uint8_t*)segment + ofs, info_size - ofs);

  // initialize pages info
  const bool is_huge = (page_kind == MI_PAGE_HUGE);
  for (size_t i = 0; i < capacity; i++) {
    mi_assert_internal(i <= 255);
    segment->pages[i].segment_idx = (uint8_t)i;
    segment->pages[i].is_committed = segment->memid.initially_committed;
    segment->pages[i].is_zero_init = segment->memid.initially_zero;
    segment->pages[i].is_huge = is_huge;
  }

  // initialize
  segment->page_kind  = page_kind;
  segment->capacity   = capacity;
  segment->page_shift = page_shift;
  segment->segment_info_size = pre_size;
  segment->thread_id  = _mi_thread_id();
  segment->cookie     = _mi_ptr_cookie(segment);

  // set protection
  mi_segment_protect(segment, true, tld->os);

  // insert in free lists for small and medium pages
  if (page_kind <= MI_PAGE_MEDIUM) {
    mi_segment_insert_in_free_queue(segment, tld);
  }

  return segment;
}


static void mi_segment_free(mi_segment_t* segment, bool force, mi_segments_tld_t* tld) {
  MI_UNUSED(force);
  mi_assert(segment != NULL);
  
  // in `mi_segment_force_abandon` we set this to true to ensure the segment's memory stays valid
  if (segment->dont_free) return;

  // don't purge as we are freeing now
  mi_segment_remove_all_purges(segment, false /* don't force as we are about to free */, tld);
  mi_segment_remove_from_free_queue(segment, tld);

  mi_assert_expensive(!mi_segment_queue_contains(&tld->small_free, segment));
  mi_assert_expensive(!mi_segment_queue_contains(&tld->medium_free, segment));
  mi_assert(segment->next == NULL);
  mi_assert(segment->prev == NULL);
  _mi_stat_decrease(&tld->stats->page_committed, segment->segment_info_size);

  // return it to the OS
  mi_segment_os_free(segment, segment->segment_size, tld);
}

/* -----------------------------------------------------------
  Free page management inside a segment
----------------------------------------------------------- */


static bool mi_segment_has_free(const mi_segment_t* segment) {
  return (segment->used < segment->capacity);
}

static bool mi_segment_page_claim(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert_internal(_mi_page_segment(page) == segment);
  mi_assert_internal(!page->segment_in_use);
  mi_page_purge_remove(page, tld);

  // check commit
  if (!mi_page_ensure_committed(segment, page, tld)) return false;

  // set in-use before doing unreset to prevent delayed reset
  page->segment_in_use = true;
  segment->used++;
  mi_assert_internal(page->segment_in_use && page->is_committed && page->used==0 && !mi_pages_purge_contains(page,tld));
  mi_assert_internal(segment->used <= segment->capacity);
  if (segment->used == segment->capacity && segment->page_kind <= MI_PAGE_MEDIUM) {
    // if no more free pages, remove from the queue
    mi_assert_internal(!mi_segment_has_free(segment));
    mi_segment_remove_from_free_queue(segment, tld);
  }
  return true;
}


/* -----------------------------------------------------------
   Free
----------------------------------------------------------- */

static void mi_segment_abandon(mi_segment_t* segment, mi_segments_tld_t* tld);

// clear page data; can be called on abandoned segments
static void mi_segment_page_clear(mi_segment_t* segment, mi_page_t* page, mi_segments_tld_t* tld)
{
  mi_assert_internal(page->segment_in_use);
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(page->is_committed);
  mi_assert_internal(mi_page_not_in_queue(page, tld));

  size_t inuse = page->capacity * mi_page_block_size(page);
  _mi_stat_decrease(&tld->stats->page_committed, inuse);
  _mi_stat_decrease(&tld->stats->pages, 1);

  page->is_zero_init = false;
  page->segment_in_use = false;

  // zero the page data, but not the segment fields and capacity, page start, and block_size (for page size calculations)
  size_t block_size = page->block_size;
  uint8_t block_size_shift = page->block_size_shift;
  uint8_t heap_tag = page->heap_tag;
  uint8_t* page_start = page->page_start;
  uint16_t capacity = page->capacity;
  uint16_t reserved = page->reserved;
  ptrdiff_t ofs = offsetof(mi_page_t,capacity);
  _mi_memzero((uint8_t*)page + ofs, sizeof(*page) - ofs);
  page->capacity = capacity;
  page->reserved = reserved;
  page->block_size = block_size;
  page->block_size_shift = block_size_shift;
  page->heap_tag = heap_tag;
  page->page_start = page_start;
  segment->used--;

  // schedule purge
  mi_segment_schedule_purge(segment, page, tld);

  page->capacity = 0;  // after purge these can be zero'd now
  page->reserved = 0;
}

void _mi_segment_page_free(mi_page_t* page, bool force, mi_segments_tld_t* tld)
{
  mi_assert(page != NULL);
  mi_segment_t* segment = _mi_page_segment(page);
  mi_assert_expensive(mi_segment_is_valid(segment,tld));
  mi_pages_try_purge(false /*force?*/, tld);

  // mark it as free now
  mi_segment_page_clear(segment, page, tld);

  if (segment->used == 0) {
    // no more used pages; remove from the free list and free the segment
    mi_segment_free(segment, force, tld);
  }
  else {
    if (segment->used == segment->abandoned) {
      // only abandoned pages; remove from free list and abandon
      mi_segment_abandon(segment,tld);
    }
    else if (segment->used + 1 == segment->capacity) {
      mi_assert_internal(segment->page_kind <= MI_PAGE_MEDIUM); // large and huge pages are always the single page in a segment
      if (segment->page_kind <= MI_PAGE_MEDIUM) {
        // move back to segments  free list
        mi_segment_insert_in_free_queue(segment,tld);
      }
    }
  }
}


/* -----------------------------------------------------------
Abandonment

When threads terminate, they can leave segments with
live blocks (reached through other threads). Such segments
are "abandoned" and will be reclaimed by other threads to
reuse their pages and/or free them eventually. The
`thread_id` of such segments is 0.

When a block is freed in an abandoned segment, the segment
is reclaimed into that thread.

Moreover, if threads are looking for a fresh segment, they
will first consider abondoned segments -- these can be found
by scanning the arena memory
(segments outside arena memoryare only reclaimed by a free).
----------------------------------------------------------- */

/* -----------------------------------------------------------
   Abandon segment/page
----------------------------------------------------------- */

static void mi_segment_abandon(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(segment->used == segment->abandoned);
  mi_assert_internal(segment->used > 0);
  mi_assert_expensive(mi_segment_is_valid(segment, tld));

  // Potentially force purge. Only abandoned segments in arena memory can be
  // reclaimed without a free so if a segment is not from an arena we force purge here to be conservative.
  mi_pages_try_purge(false /*force?*/,tld);
  const bool force_purge = (segment->memid.memkind != MI_MEM_ARENA) ||  mi_option_is_enabled(mi_option_abandoned_page_purge);
  mi_segment_remove_all_purges(segment, force_purge, tld);

  // remove the segment from the free page queue if needed
  mi_segment_remove_from_free_queue(segment, tld);
  mi_assert_internal(segment->next == NULL && segment->prev == NULL);

  // all pages in the segment are abandoned; add it to the abandoned list
  _mi_stat_increase(&tld->stats->segments_abandoned, 1);
  mi_segments_track_size(-((long)segment->segment_size), tld);
  segment->abandoned_visits = 0;
  if (segment->was_reclaimed) {
    tld->reclaim_count--;
    segment->was_reclaimed = false;
  }
  _mi_arena_segment_mark_abandoned(segment);
}

void _mi_segment_page_abandon(mi_page_t* page, mi_segments_tld_t* tld) {
  mi_assert(page != NULL);
  mi_assert_internal(mi_page_thread_free_flag(page)==MI_NEVER_DELAYED_FREE);
  mi_assert_internal(mi_page_heap(page) == NULL);
  mi_segment_t* segment = _mi_page_segment(page);
  mi_assert_expensive(!mi_pages_purge_contains(page, tld));
  mi_assert_expensive(mi_segment_is_valid(segment, tld));
  segment->abandoned++;
  _mi_stat_increase(&tld->stats->pages_abandoned, 1);
  mi_assert_internal(segment->abandoned <= segment->used);
  if (segment->used == segment->abandoned) {
    // all pages are abandoned, abandon the entire segment
    mi_segment_abandon(segment, tld);
  }
}

/* -----------------------------------------------------------
  Reclaim abandoned pages
----------------------------------------------------------- */

// Possibly clear pages and check if free space is available
static bool mi_segment_check_free(mi_segment_t* segment, size_t block_size, bool* all_pages_free)
{
  mi_assert_internal(mi_atomic_load_relaxed(&segment->thread_id) == 0);
  bool has_page = false;
  size_t pages_used = 0;
  size_t pages_used_empty = 0;
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (page->segment_in_use) {
      pages_used++;
      // ensure used count is up to date and collect potential concurrent frees
      _mi_page_free_collect(page, false);
      if (mi_page_all_free(page)) {
        // if everything free already, page can be reused for some block size
        // note: don't clear the page yet as we can only OS reset it once it is reclaimed
        pages_used_empty++;
        has_page = true;
      }
      else if (mi_page_block_size(page) == block_size && mi_page_has_any_available(page)) {
        // a page has available free blocks of the right size
        has_page = true;
      }
    }
    else {
      // whole empty page
      has_page = true;
    }
  }
  mi_assert_internal(pages_used == segment->used && pages_used >= pages_used_empty);
  if (all_pages_free != NULL) {
    *all_pages_free = ((pages_used - pages_used_empty) == 0);
  }
  return has_page;
}


// Reclaim a segment; returns NULL if the segment was freed
// set `right_page_reclaimed` to `true` if it reclaimed a page of the right `block_size` that was not full.
static mi_segment_t* mi_segment_reclaim(mi_segment_t* segment, mi_heap_t* heap, size_t requested_block_size, bool* right_page_reclaimed, mi_segments_tld_t* tld) {
  if (right_page_reclaimed != NULL) { *right_page_reclaimed = false; }
  // can be 0 still with abandoned_next, or already a thread id for segments outside an arena that are reclaimed on a free.
  mi_assert_internal(mi_atomic_load_relaxed(&segment->thread_id) == 0 || mi_atomic_load_relaxed(&segment->thread_id) == _mi_thread_id());
  mi_assert_internal(segment->subproc == heap->tld->segments.subproc); // only reclaim within the same subprocess
  mi_atomic_store_release(&segment->thread_id, _mi_thread_id());
  segment->abandoned_visits = 0;
  segment->was_reclaimed = true;
  tld->reclaim_count++;
  mi_segments_track_size((long)segment->segment_size, tld);
  mi_assert_internal(segment->next == NULL && segment->prev == NULL);
  mi_assert_expensive(mi_segment_is_valid(segment, tld));
  _mi_stat_decrease(&tld->stats->segments_abandoned, 1);

  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (page->segment_in_use) {
      mi_assert_internal(page->is_committed);
      mi_assert_internal(mi_page_not_in_queue(page, tld));
      mi_assert_internal(mi_page_thread_free_flag(page)==MI_NEVER_DELAYED_FREE);
      mi_assert_internal(mi_page_heap(page) == NULL);
      segment->abandoned--;
      mi_assert(page->next == NULL);
      _mi_stat_decrease(&tld->stats->pages_abandoned, 1);
      // get the target heap for this thread which has a matching heap tag (so we reclaim into a matching heap)
      mi_heap_t* target_heap = _mi_heap_by_tag(heap, page->heap_tag);  // allow custom heaps to separate objects
      if (target_heap == NULL) {
        target_heap = heap;
        _mi_error_message(EFAULT, "page with tag %u cannot be reclaimed by a heap with the same tag (using heap tag %u instead)\n", page->heap_tag, heap->tag );
      }
      // associate the heap with this page, and allow heap thread delayed free again.
      mi_page_set_heap(page, target_heap);
      _mi_page_use_delayed_free(page, MI_USE_DELAYED_FREE, true); // override never (after heap is set)
      _mi_page_free_collect(page, false); // ensure used count is up to date
      if (mi_page_all_free(page)) {
        // if everything free already, clear the page directly
        mi_segment_page_clear(segment, page, tld);  // reset is ok now
      }
      else {
        // otherwise reclaim it into the heap
        _mi_page_reclaim(target_heap, page);
        if (requested_block_size == mi_page_block_size(page) && mi_page_has_any_available(page) && heap == target_heap) {
          if (right_page_reclaimed != NULL) { *right_page_reclaimed = true; }
        }
      }
    }
    /* expired
    else if (page->is_committed) {  // not in-use, and not reset yet
      // note: do not reset as this includes pages that were not touched before
      // mi_pages_purge_add(segment, page, tld);
    }
    */
  }
  mi_assert_internal(segment->abandoned == 0);
  if (segment->used == 0) {
    mi_assert_internal(right_page_reclaimed == NULL || !(*right_page_reclaimed));
    mi_segment_free(segment, false, tld);
    return NULL;
  }
  else {
    if (segment->page_kind <= MI_PAGE_MEDIUM && mi_segment_has_free(segment)) {
      mi_segment_insert_in_free_queue(segment, tld);
    }
    return segment;
  }
}


// attempt to reclaim a particular segment (called from multi threaded free `alloc.c:mi_free_block_mt`)
bool _mi_segment_attempt_reclaim(mi_heap_t* heap, mi_segment_t* segment) {
  if (mi_atomic_load_relaxed(&segment->thread_id) != 0) return false;  // it is not abandoned
  if (segment->subproc != heap->tld->segments.subproc)  return false;  // only reclaim within the same subprocess
  if (!_mi_heap_memid_is_suitable(heap,segment->memid)) return false;  // don't reclaim between exclusive and non-exclusive arena's
  const long target = _mi_option_get_fast(mi_option_target_segments_per_thread);
  if (target > 0 && (size_t)target <= heap->tld->segments.count) return false; // don't reclaim if going above the target count

  // don't reclaim more from a `free` call than half the current segments
  // this is to prevent a pure free-ing thread to start owning too many segments
  // (but not for out-of-arena segments as that is the main way to be reclaimed for those)
  if (segment->memid.memkind == MI_MEM_ARENA && heap->tld->segments.reclaim_count * 2 > heap->tld->segments.count) {
    return false;
  }
  if (_mi_arena_segment_clear_abandoned(segment)) {  // atomically unabandon
    mi_segment_t* res = mi_segment_reclaim(segment, heap, 0, NULL, &heap->tld->segments);
    mi_assert_internal(res == segment);
    return (res != NULL);
  }
  return false;
}

void _mi_abandoned_reclaim_all(mi_heap_t* heap, mi_segments_tld_t* tld) {
  mi_segment_t* segment;
  mi_arena_field_cursor_t current;
  _mi_arena_field_cursor_init(heap, tld->subproc, true /* visit all, blocking */, &current);
  while ((segment = _mi_arena_segment_clear_abandoned_next(&current)) != NULL) {
    mi_segment_reclaim(segment, heap, 0, NULL, tld);
  }
  _mi_arena_field_cursor_done(&current);
}


static bool segment_count_is_within_target(mi_segments_tld_t* tld, size_t* ptarget) {
  const size_t target = (size_t)mi_option_get_clamp(mi_option_target_segments_per_thread, 0, 1024);
  if (ptarget != NULL) { *ptarget = target; }
  return (target == 0 || tld->count < target);
}

static long mi_segment_get_reclaim_tries(mi_segments_tld_t* tld) {
  // limit the tries to 10% (default) of the abandoned segments with at least 8 and at most 1024 tries.
  const size_t perc = (size_t)mi_option_get_clamp(mi_option_max_segment_reclaim, 0, 100);
  if (perc <= 0) return 0;
  const size_t total_count = mi_atomic_load_relaxed(&tld->subproc->abandoned_count);
  if (total_count == 0) return 0;
  const size_t relative_count = (total_count > 10000 ? (total_count / 100) * perc : (total_count * perc) / 100); // avoid overflow
  long max_tries = (long)(relative_count <= 1 ? 1 : (relative_count > 1024 ? 1024 : relative_count));
  if (max_tries < 8 && total_count > 8) { max_tries = 8;  }
  return max_tries;
}

static mi_segment_t* mi_segment_try_reclaim(mi_heap_t* heap, size_t block_size, mi_page_kind_t page_kind, bool* reclaimed, mi_segments_tld_t* tld)
{
  *reclaimed = false;
  long max_tries = mi_segment_get_reclaim_tries(tld);
  if (max_tries <= 0) return NULL;

  mi_segment_t* result = NULL;
  mi_segment_t* segment = NULL;
  mi_arena_field_cursor_t current;
  _mi_arena_field_cursor_init(heap, tld->subproc, false /* non-blocking */, &current);
  while (segment_count_is_within_target(tld,NULL) && (max_tries-- > 0) && ((segment = _mi_arena_segment_clear_abandoned_next(&current)) != NULL))
  {
    mi_assert(segment->subproc == heap->tld->segments.subproc); // cursor only visits segments in our sub-process
    segment->abandoned_visits++;
    // todo: should we respect numa affinity for abondoned reclaim? perhaps only for the first visit?
    // todo: an arena exclusive heap will potentially visit many abandoned unsuitable segments and use many tries
    // Perhaps we can skip non-suitable ones in a better way?
    bool is_suitable = _mi_heap_memid_is_suitable(heap, segment->memid);
    bool all_pages_free;
    bool has_page = mi_segment_check_free(segment,block_size,&all_pages_free); // try to free up pages (due to concurrent frees)
    if (all_pages_free) {
      // free the segment (by forced reclaim) to make it available to other threads.
      // note1: we prefer to free a segment as that might lead to reclaiming another
      // segment that is still partially used.
      // note2: we could in principle optimize this by skipping reclaim and directly
      // freeing but that would violate some invariants temporarily)
      mi_segment_reclaim(segment, heap, 0, NULL, tld);
    }
    else if (has_page && segment->page_kind == page_kind && is_suitable) {
      // found a free page of the right kind, or page of the right block_size with free space
      // we return the result of reclaim (which is usually `segment`) as it might free
      // the segment due to concurrent frees (in which case `NULL` is returned).
      result = mi_segment_reclaim(segment, heap, block_size, reclaimed, tld);
      break;
    }
    else if (segment->abandoned_visits > 3 && is_suitable) {
      // always reclaim on 3rd visit to limit the abandoned segment count.
      mi_segment_reclaim(segment, heap, 0, NULL, tld);
    }
    else {
      // otherwise, mark it back as abandoned
      // todo: reset delayed pages in the segment?
      _mi_arena_segment_mark_abandoned(segment);
    }
  }
  _mi_arena_field_cursor_done(&current);
  return result;
}


/* -----------------------------------------------------------
  Force abandon a segment that is in use by our thread
----------------------------------------------------------- */

// force abandon a segment
static void mi_segment_force_abandon(mi_segment_t* segment, mi_segments_tld_t* tld)
{
  mi_assert_internal(segment->abandoned < segment->used);
  mi_assert_internal(!segment->dont_free);
  
  // ensure the segment does not get free'd underneath us (so we can check if a page has been freed in `mi_page_force_abandon`)
  segment->dont_free = true;

  // for all pages
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* page = &segment->pages[i];
    if (page->segment_in_use) {
      // abandon the page if it is still in-use (this will free the page if possible as well (but not our segment))
      mi_assert_internal(segment->used > 0);
      if (segment->used == segment->abandoned+1) {
        // the last page.. abandon and return as the segment will be abandoned after this
        // and we should no longer access it.
        segment->dont_free = false;
        _mi_page_force_abandon(page);
        return;
      }
      else {
        // abandon and continue
        _mi_page_force_abandon(page);
      }
    }
  }
  segment->dont_free = false;
  mi_assert(segment->used == segment->abandoned);
  mi_assert(segment->used == 0);
  if (segment->used == 0) {  // paranoia
    // all free now
    mi_segment_free(segment, false, tld);
  }
  else {
    // perform delayed purges
    mi_pages_try_purge(false /* force? */, tld);
  }
}


// try abandon segments.
// this should be called from `reclaim_or_alloc` so we know all segments are (about) fully in use.
static void mi_segments_try_abandon_to_target(mi_heap_t* heap, size_t target, mi_segments_tld_t* tld) {
  if (target <= 1) return;
  const size_t min_target = (target > 4 ? (target*3)/4 : target);  // 75%
  // todo: we should maintain a list of segments per thread; for now, only consider segments from the heap full pages
  for (int i = 0; i < 64 && tld->count >= min_target; i++) {
    mi_page_t* page = heap->pages[MI_BIN_FULL].first;
    while (page != NULL && mi_page_is_huge(page)) {
      page = page->next;
    }
    if (page==NULL) {
      break;
    }
    mi_segment_t* segment = _mi_page_segment(page);
    mi_segment_force_abandon(segment, tld);
    mi_assert_internal(page != heap->pages[MI_BIN_FULL].first); // as it is just abandoned
  }
}

// try abandon segments.
// this should be called from `reclaim_or_alloc` so we know all segments are (about) fully in use.
static void mi_segments_try_abandon(mi_heap_t* heap, mi_segments_tld_t* tld) {
  // we call this when we are about to add a fresh segment so we should be under our target segment count.
  size_t target = 0;
  if (segment_count_is_within_target(tld, &target)) return;
  mi_segments_try_abandon_to_target(heap, target, tld);
}

void mi_collect_reduce(size_t target_size) mi_attr_noexcept {
  mi_collect(true);
  mi_heap_t* heap = mi_heap_get_default();
  mi_segments_tld_t* tld = &heap->tld->segments;
  size_t target = target_size / MI_SEGMENT_SIZE;
  if (target == 0) {
    target = (size_t)mi_option_get_clamp(mi_option_target_segments_per_thread, 1, 1024);
  }
  mi_segments_try_abandon_to_target(heap, target, tld);
}

/* -----------------------------------------------------------
   Reclaim or allocate
----------------------------------------------------------- */

static mi_segment_t* mi_segment_reclaim_or_alloc(mi_heap_t* heap, size_t block_size, mi_page_kind_t page_kind, size_t page_shift, mi_segments_tld_t* tld, mi_os_tld_t* os_tld)
{
  mi_assert_internal(page_kind <= MI_PAGE_LARGE);
  mi_assert_internal(block_size <= MI_LARGE_OBJ_SIZE_MAX);

  // try to abandon some segments to increase reuse between threads
  mi_segments_try_abandon(heap,tld);

  // 1. try to reclaim an abandoned segment
  bool reclaimed;
  mi_segment_t* segment = mi_segment_try_reclaim(heap, block_size, page_kind, &reclaimed, tld);
  mi_assert_internal(segment == NULL || _mi_arena_memid_is_suitable(segment->memid, heap->arena_id));
  if (reclaimed) {
    // reclaimed the right page right into the heap
    mi_assert_internal(segment != NULL && segment->page_kind == page_kind && page_kind <= MI_PAGE_LARGE);
    return NULL; // pretend out-of-memory as the page will be in the page queue of the heap with available blocks
  }
  else if (segment != NULL) {
    // reclaimed a segment with empty pages (of `page_kind`) in it
    return segment;
  }
  // 2. otherwise allocate a fresh segment
  return mi_segment_alloc(0, page_kind, page_shift, 0, heap->arena_id, tld, os_tld);
}


/* -----------------------------------------------------------
   Small page allocation
----------------------------------------------------------- */

static mi_page_t* mi_segment_find_free(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(mi_segment_has_free(segment));
  mi_assert_expensive(mi_segment_is_valid(segment, tld));
  for (size_t i = 0; i < segment->capacity; i++) {  // TODO: use a bitmap instead of search?
    mi_page_t* page = &segment->pages[i];
    if (!page->segment_in_use) {
      bool ok = mi_segment_page_claim(segment, page, tld);
      if (ok) return page;
    }
  }
  mi_assert(false);
  return NULL;
}

// Allocate a page inside a segment. Requires that the page has free pages
static mi_page_t* mi_segment_page_alloc_in(mi_segment_t* segment, mi_segments_tld_t* tld) {
  mi_assert_internal(mi_segment_has_free(segment));
  return mi_segment_find_free(segment, tld);
}

static mi_page_t* mi_segment_page_try_alloc_in_queue(mi_heap_t* heap, mi_page_kind_t kind, mi_segments_tld_t* tld) {
  // find an available segment the segment free queue
  mi_segment_queue_t* const free_queue = mi_segment_free_queue_of_kind(kind, tld);
  for (mi_segment_t* segment = free_queue->first; segment != NULL; segment = segment->next) {
    if (_mi_arena_memid_is_suitable(segment->memid, heap->arena_id) && mi_segment_has_free(segment)) {
      return mi_segment_page_alloc_in(segment, tld);
    }
  }
  return NULL;
}

static mi_page_t* mi_segment_page_alloc(mi_heap_t* heap, size_t block_size, mi_page_kind_t kind, size_t page_shift, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  mi_page_t* page = mi_segment_page_try_alloc_in_queue(heap, kind, tld);
  if (page == NULL) {
    // possibly allocate or reclaim a fresh segment
    mi_segment_t* const segment = mi_segment_reclaim_or_alloc(heap, block_size, kind, page_shift, tld, os_tld);
    if (segment == NULL) return NULL;  // return NULL if out-of-memory (or reclaimed)
    mi_assert_internal(segment->page_kind==kind);
    mi_assert_internal(segment->used < segment->capacity);
    mi_assert_internal(_mi_arena_memid_is_suitable(segment->memid, heap->arena_id));
    page = mi_segment_page_try_alloc_in_queue(heap, kind, tld);  // this should now succeed
  }
  mi_assert_internal(page != NULL);
  #if MI_DEBUG>=2 && !MI_TRACK_ENABLED // && !MI_TSAN
  // verify it is committed
  mi_segment_raw_page_start(_mi_page_segment(page), page, NULL)[0] = 0;
  #endif
  return page;
}

static mi_page_t* mi_segment_small_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  return mi_segment_page_alloc(heap, block_size, MI_PAGE_SMALL,MI_SMALL_PAGE_SHIFT,tld,os_tld);
}

static mi_page_t* mi_segment_medium_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  return mi_segment_page_alloc(heap, block_size, MI_PAGE_MEDIUM, MI_MEDIUM_PAGE_SHIFT, tld, os_tld);
}

/* -----------------------------------------------------------
   large page allocation
----------------------------------------------------------- */

static mi_page_t* mi_segment_large_page_alloc(mi_heap_t* heap, size_t block_size, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  mi_segment_t* segment = mi_segment_reclaim_or_alloc(heap,block_size,MI_PAGE_LARGE,MI_LARGE_PAGE_SHIFT,tld,os_tld);
  if (segment == NULL) return NULL;
  mi_page_t* page = mi_segment_find_free(segment, tld);
  mi_assert_internal(page != NULL);
#if MI_DEBUG>=2 && !MI_TRACK_ENABLED // && !MI_TSAN
  mi_segment_raw_page_start(segment, page, NULL)[0] = 0;
#endif
  return page;
}

static mi_page_t* mi_segment_huge_page_alloc(size_t size, size_t page_alignment, mi_arena_id_t req_arena_id, mi_segments_tld_t* tld, mi_os_tld_t* os_tld)
{
  mi_segment_t* segment = mi_segment_alloc(size, MI_PAGE_HUGE, MI_SEGMENT_SHIFT + 1, page_alignment, req_arena_id, tld, os_tld);
  if (segment == NULL) return NULL;
  mi_assert_internal(mi_segment_page_size(segment) - segment->segment_info_size - (2*(MI_SECURE == 0 ? 0 : _mi_os_page_size())) >= size);
  #if MI_HUGE_PAGE_ABANDON
  segment->thread_id = 0; // huge pages are immediately abandoned
  mi_segments_track_size(-(long)segment->segment_size, tld);
  #endif
  mi_page_t* page = mi_segment_find_free(segment, tld);
  mi_assert_internal(page != NULL);
  mi_assert_internal(page->is_huge);

  // for huge pages we initialize the block_size as we may
  // overallocate to accommodate large alignments.
  size_t psize;
  uint8_t* start = mi_segment_raw_page_start(segment, page, &psize);
  page->block_size = psize;

  // reset the part of the page that will not be used; this can be quite large (close to MI_SEGMENT_SIZE)
  if (page_alignment > 0 && segment->allow_decommit && page->is_committed) {
    uint8_t* aligned_p = (uint8_t*)_mi_align_up((uintptr_t)start, page_alignment);
    mi_assert_internal(_mi_is_aligned(aligned_p, page_alignment));
    mi_assert_internal(psize - (aligned_p - start) >= size);
    uint8_t* decommit_start = start + sizeof(mi_block_t); // for the free list
    ptrdiff_t decommit_size = aligned_p - decommit_start;
    _mi_os_reset(decommit_start, decommit_size, os_tld->stats);  // do not decommit as it may be in a region
  }

  return page;
}

#if MI_HUGE_PAGE_ABANDON
// free huge block from another thread
void _mi_segment_huge_page_free(mi_segment_t* segment, mi_page_t* page, mi_block_t* block) {
  // huge page segments are always abandoned and can be freed immediately by any thread
  mi_assert_internal(segment->page_kind==MI_PAGE_HUGE);
  mi_assert_internal(segment == _mi_page_segment(page));
  mi_assert_internal(mi_atomic_load_relaxed(&segment->thread_id)==0);

  // claim it and free
  mi_heap_t* heap = mi_heap_get_default(); // issue #221; don't use the internal get_default_heap as we need to ensure the thread is initialized.
  // paranoia: if this it the last reference, the cas should always succeed
  size_t expected_tid = 0;
  if (mi_atomic_cas_strong_acq_rel(&segment->thread_id, &expected_tid, heap->thread_id)) {
    mi_block_set_next(page, block, page->free);
    page->free = block;
    page->used--;
    page->is_zero_init = false;
    mi_assert(page->used == 0);
    mi_tld_t* tld = heap->tld;
    mi_segments_track_size((long)segment->segment_size, &tld->segments);
    _mi_segment_page_free(page, true, &tld->segments);
  }
#if (MI_DEBUG!=0)
  else {
    mi_assert_internal(false);
  }
#endif
}

#else
// reset memory of a huge block from another thread
void _mi_segment_huge_page_reset(mi_segment_t* segment, mi_page_t* page, mi_block_t* block) {
  mi_assert_internal(segment->page_kind == MI_PAGE_HUGE);
  mi_assert_internal(segment == _mi_page_segment(page));
  mi_assert_internal(page->used == 1); // this is called just before the free
  mi_assert_internal(page->free == NULL);
  if (segment->allow_decommit && page->is_committed) {
    size_t usize = mi_usable_size(block);
    if (usize > sizeof(mi_block_t)) {
      usize = usize - sizeof(mi_block_t);
      uint8_t* p = (uint8_t*)block + sizeof(mi_block_t);
      _mi_os_reset(p, usize, &_mi_stats_main);
    }
  }
}
#endif

/* -----------------------------------------------------------
   Page allocation
----------------------------------------------------------- */

mi_page_t* _mi_segment_page_alloc(mi_heap_t* heap, size_t block_size, size_t page_alignment, mi_segments_tld_t* tld, mi_os_tld_t* os_tld) {
  mi_page_t* page;
  if mi_unlikely(page_alignment > MI_BLOCK_ALIGNMENT_MAX) {
    mi_assert_internal(_mi_is_power_of_two(page_alignment));
    mi_assert_internal(page_alignment >= MI_SEGMENT_SIZE);
    //mi_assert_internal((MI_SEGMENT_SIZE % page_alignment) == 0);
    if (page_alignment < MI_SEGMENT_SIZE) { page_alignment = MI_SEGMENT_SIZE; }
    page = mi_segment_huge_page_alloc(block_size, page_alignment, heap->arena_id, tld, os_tld);
  }
  else if (block_size <= MI_SMALL_OBJ_SIZE_MAX) {
    page = mi_segment_small_page_alloc(heap, block_size, tld, os_tld);
  }
  else if (block_size <= MI_MEDIUM_OBJ_SIZE_MAX) {
    page = mi_segment_medium_page_alloc(heap, block_size, tld, os_tld);
  }
  else if (block_size <= MI_LARGE_OBJ_SIZE_MAX /* || mi_is_good_fit(block_size, MI_LARGE_PAGE_SIZE - sizeof(mi_segment_t)) */ ) {
    page = mi_segment_large_page_alloc(heap, block_size, tld, os_tld);
  }
  else {
    page = mi_segment_huge_page_alloc(block_size, page_alignment, heap->arena_id, tld, os_tld);
  }
  mi_assert_expensive(page == NULL || mi_segment_is_valid(_mi_page_segment(page),tld));
  mi_assert_internal(page == NULL || (mi_segment_page_size(_mi_page_segment(page)) - (MI_SECURE == 0 ? 0 : _mi_os_page_size())) >= block_size);
  // mi_segment_try_purge(tld);
  mi_assert_internal(page == NULL || mi_page_not_in_queue(page, tld));
  mi_assert_internal(page == NULL || _mi_page_segment(page)->subproc == tld->subproc);
  return page;
}


/* -----------------------------------------------------------
   Visit blocks in a segment (only used for abandoned segments)
----------------------------------------------------------- */

static bool mi_segment_visit_page(mi_page_t* page, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  mi_heap_area_t area;
  _mi_heap_area_init(&area, page);
  if (!visitor(NULL, &area, NULL, area.block_size, arg)) return false;
  if (visit_blocks) {
    return _mi_heap_area_visit_blocks(&area, page, visitor, arg);
  }
  else {
    return true;
  }
}

bool _mi_segment_visit_blocks(mi_segment_t* segment, int heap_tag, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  for (size_t i = 0; i < segment->capacity; i++) {
    mi_page_t* const page = &segment->pages[i];
    if (page->segment_in_use) {
      if (heap_tag < 0 || (int)page->heap_tag == heap_tag) {
        if (!mi_segment_visit_page(page, visit_blocks, visitor, arg)) return false;
      }
    }
  }
  return true;
}
