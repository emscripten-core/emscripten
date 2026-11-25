/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "mimalloc.h"
#include "mimalloc/prim/prim-tls.h"
#include "mimalloc/util/stats.h"
#include "mimalloc/alloc/page.h"
#include "mimalloc/profile/sample-guarded.h"
#include "mimalloc/profile/sample-profile.h"

//----------------------------------------------------------------------------
// General sampled allocation (called from `alloc/alloc-generic.c:mi_malloc_generic_fallback`)
// This is for both guarded and profiled sampling
//----------------------------------------------------------------------------

size_t _mi_theap_update_sample_rate(mi_theap_t* theap) {
  const size_t old_sample_rate = theap->sample_rate;
  theap->sample_rate = theap->profile_sample_rate;
  if (theap->sample_rate == 0 || (theap->guarded_sample_rate!=0 && theap->sample_rate > theap->guarded_sample_rate)) {
    theap->sample_rate = theap->guarded_sample_rate;
  }
  if (theap->sample_countdown > theap->sample_rate) {
    theap->sample_countdown = theap->sample_rate;  // todo: adjust difference?
  }
  return old_sample_rate;
}

// Restart the sample countdown at the nearest profile or guarded countdown.
// These bytes are counted down from both in advance, so the next sample only counts the bytes beyond it.
static void mi_theap_sample_countdown_restart(mi_theap_t* theap) {
  size_t countdown = theap->sample_rate;
  if (theap->profile_sample_rate!=0 && theap->profile_sample_countdown < countdown) { countdown = theap->profile_sample_countdown; }
  if (theap->guarded_sample_rate!=0 && theap->guarded_sample_countdown < countdown) { countdown = theap->guarded_sample_countdown; }
  if (theap->profile_sample_rate!=0) { theap->profile_sample_countdown -= countdown; }
  if (theap->guarded_sample_rate!=0) { theap->guarded_sample_countdown -= countdown; }
  theap->sample_countdown = countdown;
}

// The bytes beyond a derived countdown that were counted in a batch before this allocation (and count towards the next sample).
// This is at most the `rate` so any bytes beyond that are attributed to the current sample (instead of being lost).
static uint64_t mi_sample_carry(uint64_t excess, size_t req_size, size_t rate) {
  const uint64_t carry = (excess > req_size ? excess - req_size : 0);
  return (carry < rate ? carry : rate);
}

static size_t mi_sample_countdown_next(size_t rate, uint64_t carry) {
  return (carry < rate ? rate - (size_t)carry : 0);
}

// Called when the theap sample countdown <= req_size; should call guarded or profiled allocation.
static void* mi_theap_malloc_sampled_ex(mi_theap_t* theap, size_t req_size, bool zero, mi_page_t** ppage)
{
  // the size has not yet been counted against the countdown (and does not include MI_PADDING_SIZE)
  mi_assert_internal(req_size <= MI_MAX_ALLOC_SIZE);
  mi_assert_internal((theap->sample_countdown==SIZE_MAX && _mi_is_empty_theap(theap)) ||
                     (theap->sample_countdown <= MI_SAMPLE_COUNTDOWN_MAX && theap->sample_countdown < req_size));
  const size_t size = req_size + MI_PADDING_SIZE;

  // handle empty theap and disabled sampling
  if (theap->sample_rate==0) {
    if (!_mi_is_empty_theap(theap)) {                    // avoid writing to the initial empty theap
      theap->sample_countdown = MI_SAMPLE_COUNTDOWN_MAX; // avoid the sampling path for a long time
    }
    return _mi_malloc_generic_no_sample(theap,size,zero,ppage);
  }
  // never sample allocations made by a profiler `on_alloc` callback (e.g. by `backtrace`) to avoid recursion
  if mi_unlikely(theap->tld->in_profiler) {
    return _mi_malloc_generic_no_sample(theap,size,zero,ppage);
  }

  mi_assert_internal(!_mi_is_empty_theap(theap));
  mi_assert_internal(req_size <= SIZE_MAX/2);
  mi_assert_internal(theap->sample_rate > 0);
  mi_assert_internal(theap->sample_rate <= SIZE_MAX/2);
  mi_assert_internal(theap->sample_rate >= theap->sample_countdown);
  mi_assert_internal(theap->profile_sample_rate!=0 || theap->profile_sample_countdown==0);
  mi_assert_internal(theap->guarded_sample_rate!=0 || theap->guarded_sample_countdown==0);

  // the requested bytes beyond the countdown (which was already counted down from the derived countdowns at restart)
  const uint64_t requested = (uint64_t)(req_size - theap->sample_countdown) + theap->sample_requested;
  theap->sample_countdown = 0;
  theap->sample_requested = 0;

  // update the derived countdowns; a sample is due if the requested bytes exceed its countdown
  uint64_t profile_excess = 0;
  uint64_t guarded_excess = 0;
  uint64_t profile_requested = 0;
  if (theap->profile_sample_rate!=0) {
    profile_requested = (theap->profile_sample_rate - theap->profile_sample_countdown) + requested;  // since the last profile sample
    if (requested <= theap->profile_sample_countdown) { theap->profile_sample_countdown -= (size_t)requested; } else { profile_excess = requested - theap->profile_sample_countdown; }
  }
  if (theap->guarded_sample_rate!=0) {
    if (requested <= theap->guarded_sample_countdown) { theap->guarded_sample_countdown -= (size_t)requested; } else { guarded_excess = requested - theap->guarded_sample_countdown; }
  }

  void* p;
  if (profile_excess > 0) {
    const uint64_t carry = mi_sample_carry(profile_excess, req_size, theap->profile_sample_rate);
    p = _mi_theap_malloc_profiled(theap,size,profile_requested - carry,zero,ppage);  // can update the profile sample rate
    theap->profile_sample_countdown = mi_sample_countdown_next(theap->profile_sample_rate, carry);
    if (guarded_excess > 0) { theap->guarded_sample_countdown = 0; }  // retry at the next allocation
  }
  else if (guarded_excess > 0) {
    if (req_size >= theap->guarded_size_min && req_size <= theap->guarded_size_max) {
      p = _mi_theap_malloc_guarded(theap,size,zero,ppage);
      theap->guarded_sample_countdown = mi_sample_countdown_next(theap->guarded_sample_rate, mi_sample_carry(guarded_excess, req_size, theap->guarded_sample_rate));
    }
    else {
      p = _mi_malloc_generic_no_sample(theap,size,zero,ppage);
      theap->guarded_sample_countdown = 0;  // retry at the next allocation
    }
  }
  else {
    p = _mi_malloc_generic_no_sample(theap,size,zero,ppage);
  }
  mi_theap_sample_countdown_restart(theap);
  return p;
}

mi_decl_noinline mi_decl_restrict void* _mi_theap_malloc_sampled(mi_theap_t* theap, size_t req_size, bool zero, mi_page_t** ppage) mi_attr_noexcept
{
  void* const p = mi_theap_malloc_sampled_ex(theap, req_size, zero, ppage);
  if mi_unlikely(p==NULL) { errno = ENOMEM; }  // as in `alloc/alloc-generic.c:mi_malloc_generic_fallback`
  return p;
}



//----------------------------------------------------------------------------
// Util
//----------------------------------------------------------------------------

static mi_profiler_t* mi_heap_profiler(const mi_heap_t* heap) {
  return mi_atomic_load_ptr_acquire(mi_profiler_t,&heap->profiler);
}

static mi_profiler_t* mi_theap_get_enabled_profiler(const mi_theap_t* theap) {
  mi_heap_t* const heap = mi_theap_heap(theap);
  mi_profiler_t* prof = mi_atomic_load_ptr_acquire(mi_profiler_t, &heap->profiler);
  if (prof!=NULL && mi_profiler_is_enabled(prof) && prof->on_alloc != NULL) {
    return prof;
  }
  else {
    return NULL;
  }
}

//----------------------------------------------------------------------------
// Profile an allocation
//-----------------------------------------------------------------------------

// The header of a profiled block (followed by the user data of the sample data, and then the user object)
typedef struct mi_profiled_block_s {
  mi_block_t                block;          // `next` is `MI_BLOCK_TAG_PROFILED`
  size_t                    profiler_state; // the profiler state when sampled; `on_free` is only called if it is still the same generation
  mi_profiler_sample_data_t sample_data;
} mi_profiled_block_t;

// call `on_alloc`; allocations made by the profiler itself (e.g. by `backtrace`) are not sampled
static size_t mi_profiler_on_alloc(mi_theap_t* theap, mi_profiler_t* prof, mi_profiler_sample_data_t* data, void* p, size_t req_size, uint64_t requested_since_last_sample) {
  mi_tld_t* const tld = theap->tld;
  tld->in_profiler = true;
  const size_t new_sample_rate = (*prof->on_alloc)(prof, data, p, req_size, theap->profile_sample_rate, requested_since_last_sample, mi_theap_heap(theap));
  tld->in_profiler = false;
  return new_sample_rate;
}

mi_decl_noinline mi_decl_restrict void* _mi_theap_malloc_profiled(mi_theap_t* theap, size_t size, uint64_t requested_since_last_sample, bool zero, mi_page_t** ppage) mi_attr_noexcept
{
  mi_assert_internal(theap!=NULL);
  mi_assert_internal(size>=MI_PADDING_SIZE);
  const size_t req_size = size - MI_PADDING_SIZE;
  mi_profiler_t* const prof = mi_theap_get_enabled_profiler(theap);
  if (prof == NULL) { return _mi_malloc_generic_no_sample(theap,size,zero,ppage); }
  mi_assert_internal(prof!=NULL && prof->on_alloc!=NULL && mi_profiler_is_enabled(prof));

  void* p = NULL;
  size_t new_sample_rate = 0;
  if (prof->on_free==NULL || prof->sample_data_size==0) {
    // just allocate without profiler data
    p = _mi_malloc_generic_no_sample(theap,size,zero,ppage);
    if (p==NULL) { return p; }
    if (prof->on_alloc!=NULL) {
      // we are just allocation profiling (not heap profiling as on_free == NULL)
      new_sample_rate = mi_profiler_on_alloc(theap, prof, NULL /* no data */, p, req_size, requested_since_last_sample);
    }
  }
  else {
    // overallocate a larger block to store the profiler data
    // [MI_BLOCK_TAG_PROFILED] [profiler state] [ ... profile data ... ] [... user data ...]
    const size_t sample_user_data_size = mi_align_up(prof->sample_data_size > MI_PROFILE_SAMPLE_DATA_MAX_SIZE ? MI_PROFILE_SAMPLE_DATA_MAX_SIZE : prof->sample_data_size, sizeof(void*));
    const size_t header_size           = sizeof(mi_profiled_block_t) + sample_user_data_size;  // one void* too many just in case
    const size_t user_offset           = mi_align_up(header_size, MI_MAX_ALIGN_SIZE);
    const size_t oversize              = user_offset + size;
    mi_page_t* page = NULL;
    mi_profiled_block_t* const pblock = (mi_profiled_block_t*)_mi_malloc_generic_no_sample(theap,oversize,zero,&page);
    if (pblock==NULL) return NULL;
    mi_block_t* const block = &pblock->block;
    mi_assert_internal(page!=NULL);
    if (ppage!=NULL) { *ppage = page; }
    #if MI_PAGE_META_SMALL_IS_ALIGNED
    // we should never allocate something allocated as small in a non-small page or otherwise aligned mi_free_small may fail.
    // (that is why we need to limit the profiler_data_size as well)
    // we should never allocate something allocated as small in a non-small page or otherwise aligned mi_free_small may fail.
    if (size <= MI_SMALL_SIZE_MAX) { mi_assert_internal(mi_page_block_size(page) <= MI_SMALL_MAX_OBJ_SIZE); }
    #endif

    // Set up the profiled block as an interior pointer so the interior "slow" path is taken on mi_free (where we catch it to call on_free)
    mi_page_set_has_interior_pointers(page, true);
    mi_atomic_store_relaxed(&block->next, MI_BLOCK_TAG_PROFILED);
    p = mi_ptr_bounded((uint8_t*)block + user_offset, mi_bounds_user_size(req_size));  // exclude the profile data in front
    pblock->profiler_state = mi_profiler_state(prof);
    mi_profiler_sample_data_t* const sample_data = &pblock->sample_data;
    sample_data->user_data_size = sample_user_data_size;

    // and call the profiler on_alloc
    if (prof->on_alloc!=NULL) {
      new_sample_rate = mi_profiler_on_alloc(theap, prof, sample_data, p, req_size, requested_since_last_sample);
    }
  }
  if (new_sample_rate!=0 && new_sample_rate != (size_t)theap->profile_sample_rate) {
    _mi_theap_set_profile_sample_rate(theap,new_sample_rate);
  }
  mi_theap_stat_counter_increase(theap,profile_samples,1);
  return p;
}

void _mi_page_profile_on_free(mi_page_t* page, mi_block_t* block, void* p) {
  mi_assert_internal(mi_block_ptr_is_sampled(block,p));

  // get the heap and profiler
  mi_heap_t* const heap = mi_page_heap(page);
  if (heap==NULL) return;
  mi_profiler_t* prof = mi_heap_profiler(heap);
  if (prof==NULL || prof->on_free==NULL) return;

  // only call `on_free` if the block was sampled by this profiler since it was last attached (even if it is stopped now)
  // (as the sample data may otherwise refer to deallocated data from a previous profiler)
  mi_profiled_block_t* const pblock = (mi_profiled_block_t*)block;
  if ((pblock->profiler_state >> 1) != (mi_profiler_state(prof) >> 1)) return;  // compare generations
  (*prof->on_free)(prof, &pblock->sample_data, p, heap);
}


//----------------------------------------------------------------------------
// Profiling API
//-----------------------------------------------------------------------------*

size_t _mi_theap_set_profile_sample_rate(mi_theap_t* theap, size_t sample_rate) {
  const size_t old_sample_rate = theap->profile_sample_rate;
  theap->profile_sample_rate = (sample_rate > MI_SAMPLE_RATE_MAX ? MI_SAMPLE_RATE_MAX : sample_rate);
  _mi_theap_update_sample_rate(theap);
  // The shared countdown is already deducted from the profile countdown.
  const size_t max_countdown = mi_sample_countdown_next(theap->profile_sample_rate, theap->sample_countdown);
  if (theap->profile_sample_countdown > max_countdown) {
    theap->profile_sample_countdown = max_countdown;
  }
  return old_sample_rate;
}

static const mi_profiler_t mi_nosample_profiler = { NULL, 0, MI_SAMPLE_RATE_MAX, NULL, NULL, NULL, NULL };

static bool mi_heap_set_profiler(mi_heap_t* heap, mi_profiler_t* profiler) {
  if (!mi_heap_handle_is_valid(heap)) return false;
  mi_profiler_t* previous = (profiler==NULL ? mi_atomic_load_ptr_acquire(mi_profiler_t,&heap->profiler) : NULL); // don't overwrite unless it is NULL
  if (previous == &mi_nosample_profiler) return false;
  return mi_atomic_cas_ptr_strong_release(mi_profiler_t, &heap->profiler, &previous, profiler);
}

// Called when a stopped profiler is attached: the fresh generation ensures `on_free` is
// only called for allocations that are sampled by this profiler after it was attached.
static void mi_profiler_new_generation(mi_profiler_t* profiler) {
  if (profiler==NULL) return;
  const size_t generation = mi_atomic_increment_relaxed(&_mi_process.profiler_generation) + 1;
  mi_atomic_store_release(mi_profiler_pstate(profiler), generation << 1);
}

// Attach (or detach if `profiler==NULL`) a profiler to a heap or subprocess.
// The new generation must be set before the profiler is published, as otherwise a concurrent free
// could call `on_free` for a block sampled during a previous attachment (with the same generation).
static bool mi_profiler_attach(_Atomic(mi_profiler_t*)* slot, mi_profiler_t* profiler) {
  mi_profiler_stop(profiler);
  mi_profiler_t* expected = NULL;
  if (profiler==NULL) {
    expected = mi_atomic_load_ptr_acquire(mi_profiler_t,slot);
  }
  else {
    if (mi_atomic_load_ptr_relaxed(mi_profiler_t,slot) != NULL) return false;  // don't overwrite unless it is NULL
    mi_profiler_new_generation(profiler);
  }
  return mi_atomic_cas_ptr_strong_release(mi_profiler_t, slot, &expected, profiler);
}

mi_decl_export bool mi_heap_profile(mi_heap_t* heap, mi_profiler_t* profiler) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  if (!mi_heap_handle_is_valid(heap)) return false;
  return mi_profiler_attach(&heap->profiler, profiler);
}

mi_decl_export void mi_heap_profile_disable(mi_heap_t* heap) {
  // if (heap==NULL) { heap = mi_heap_main(); }
  mi_heap_set_profiler(heap,NULL);                   // first clear any previous one
  mi_heap_set_profiler(heap,(mi_profiler_t*)&mi_nosample_profiler);  // and then install the nosample one
}

mi_decl_export bool mi_subproc_profile(mi_subproc_id_t subproc_id, mi_profiler_t* profiler) {
  mi_subproc_t* subproc = _mi_subproc_from_id(subproc_id);
  if (subproc==NULL) return false;
  if (!mi_profiler_attach(&subproc->profiler, profiler)) { return false; }
  mi_lock(&subproc->heaps_lock) {
    for (mi_heap_t* heap = subproc->heaps; heap!=NULL; heap = heap->next) {
      mi_heap_set_profiler(heap,profiler);
    }
  }
  return true;
}

mi_decl_export bool mi_profile( mi_profiler_t* profiler) {
  return mi_subproc_profile(mi_subproc_main(),profiler);
}

bool mi_profiler_start(mi_profiler_t* profiler ) {
  if (profiler==NULL) return false;
  const bool was_running = ((mi_atomic_or_acq_rel(mi_profiler_pstate(profiler), (size_t)1) & 1) != 0);
  if (was_running) return true;
  // for the main heap, if this is the profiler, start the theap more aggressively
  // otherwise it will be picked up when theaps take the slow generic malloc path.
  mi_heap_t* heap = mi_heap_main();
  if (mi_heap_profiler(heap)==profiler) {
    mi_theap_t* theap = _mi_heap_theap_peek(heap);
    if (theap!=NULL) {
      _mi_theap_set_profile_sample_rate(theap,1);
    }
  }
  return false;
}

bool mi_profiler_stop(mi_profiler_t* profiler) {
  if (profiler==NULL) return false;
  return ((mi_atomic_and_acq_rel(mi_profiler_pstate(profiler), ~(size_t)1) & 1) != 0);
}

void mi_profiler_snapshot(mi_profiler_t* profiler) {
  if (profiler==NULL) return;
  if (profiler->on_snapshot==NULL) return;
  mi_atomic_guard(&_mi_process.profile_snapshot_guard) {
    (*profiler->on_snapshot)(profiler);
  }
}
