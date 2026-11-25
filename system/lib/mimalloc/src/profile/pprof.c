/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

// ---------------------------------------------------------------------------
// This implements a basic memory profiler that outputs pprof-compatible heap
// snapshots (text or protobuf). It is basic, but it does scale with a lock-free
// hash table for locations, and uses poisson distributed sampling.
// It has no impact on runtime performance when profiling is not active.
//
// It can be used programmatically (using `mi_pprof_profiler_new` etc.)
// or be invoked with environment variables. For example:
//   MIMALLOC_PROFILE=profile ./my_program
// and then analyze the generated profile using pprof tools:
//   pprof -http=:8080 ./myprogram  profile.*
// ---------------------------------------------------------------------------

#include "mimalloc.h"
#include "mimalloc/prim/prim-tls.h"   // _mi_theap_default
#include "mimalloc/prim/prim-mte.h"   // mi_ptr_memtag_meta
#include "mimalloc/util/libc.h"
#include "mimalloc/util/options.h"
#include "mimalloc/util/stats.h"
#include "mimalloc/profile/pprof.h"
#include "mimalloc/profile/sample-profile.h"  // MI_SAMPLE_RATE_MAX
#include "mimalloc/heap/process.h"  // mi_process_get_sealed()->profile_env_profiler
#include "mimalloc/prim/os.h"       // _mi_os_is_elevated
#include "pprof.h"

// ---------------------------------------------------------------------------
// Internal API
// ---------------------------------------------------------------------------

static bool    mi_locations_init(mi_heap_t* heap, mi_locations_t* locations);
static void    mi_locations_done(mi_heap_t* heap, mi_locations_t* locations);
static mi_location_t* mi_locations_find_or_insert(mi_heap_t* heap, mi_locations_t* locations, mi_thread_id_t thread_id, const mi_callstack_t* callstack, size_t callstack_hash);

static inline mi_pprof_profiler_t* downcast( mi_profiler_t* prof ) {
  return (mi_pprof_profiler_t*)prof;
}


// ---------------------------------------------------------------------------
// Automatic profiler driven by the `MIMALLOC_PROFILE` environment variable.
// This allows profiling an arbitrary program (with no code changes) by
// overriding its allocator with a mimalloc shared library build and setting
// `MIMALLOC_PROFILE=<base_file_name>`
// (and optionally `MIMALLOC_PROFILE_ALLOC_INTERVAL=<bytes>` to periodically snapshot
// while the program is running).
// ---------------------------------------------------------------------------

#if MI_PROFILE_USE_BUILTIN

// Called once at process initialization (see `mi_process_init` in `heap/process.c`).
void _mi_pprof_profiler_init(void) {
  #if MI_PROFILE
  char fname[1024];
  if (_mi_getenv("mimalloc_profile", fname, sizeof(fname)) != 0 || fname[0] == 0) return;  // `MIMALLOC_PROFILE` not set
  if (MI_SECURE_ENV || MI_SECURE > 0 || _mi_os_is_elevated()) {
    _mi_warning_message("MIMALLOC_PROFILE is ignored in %s\n",
                        (MI_SECURE_ENV ? "secure environment mode" : (MI_SECURE > 0 ? "secure mode" : "an elevated process")));
    return;
  }
  const size_t sample_rate   = mi_option_get_size(mi_option_profile_sample_rate);           // 512 KiB by default
  const size_t alloc_interval_size = mi_option_get_size(mi_option_profile_alloc_interval);        // 1 GiB by default
  const size_t inuse_interval_size = mi_option_get_size(mi_option_profile_inuse_interval);        // 0 by default (no automatic interval snapshots)
  const size_t time_interval_secs = mi_option_get_clamp(mi_option_profile_time_interval,0,24*60*60L);  // 0 by default (no automatic interval snapshots)
  mi_profiler_t* profiler = mi_pprof_profiler_new(sample_rate, fname, alloc_interval_size, inuse_interval_size, time_interval_secs);
  if (profiler == NULL) return;
  mi_process_sealed_t* const sealed = _mi_process_unseal();
  sealed->profile_env_profiler = (mi_profiler_t*)mi_ptr_memtag_meta(profiler);  // store without memory tag
  _mi_process_seal();
  _mi_verbose_message("pprof profiler initialized with base file name: %s\n", fname);
  mi_profile(profiler);           // attach to the main sub-process (and any current/future heaps in it)
  mi_profiler_start(profiler);
  #endif
}

// Called once at process termination (see `mi_process_done` in `heap/process.c`).
void _mi_pprof_profiler_done(void) {
  #if MI_PROFILE
  mi_profiler_t* profiler = (mi_profiler_t*)mi_meta_load_maybe(mi_process_get_sealed()->profile_env_profiler);
  if (profiler == NULL) return;
  mi_process_sealed_t* const sealed = _mi_process_unseal();
  sealed->profile_env_profiler = NULL;
  _mi_process_seal();
  mi_profiler_stop(profiler);
  _mi_verbose_message("pprof profiler done\n");
  mi_profiler_snapshot(profiler);    // final snapshot so short-lived processes still get a complete profile
  mi_profile(NULL);                  // detach from the main sub-process
  // note: we do not delete the profiler as other threads may still be running and concurrently call `on_free`
  #endif
}

#else

void _mi_pprof_profiler_init(void) {
}

void _mi_pprof_profiler_done(void) {
}

#endif

// ---------------------------------------------------------------------------
// Get a location
// ---------------------------------------------------------------------------

static mi_decl_noinline mi_location_t* mi_location_get(mi_pprof_profiler_t* prof) {
  void* frames[MI_MAX_BACKTRACE_DEPTH + 1 + MI_BACKTRACE_SKIP_FRAMES];  // extra room: `_mi_pprof_backtrace` captures+shifts out its own frame and `MI_BACKTRACE_SKIP_FRAMES` wrapper frames in-place
  size_t callstack_hash = 0;
  const size_t depth = _mi_pprof_backtrace(frames, MI_MAX_BACKTRACE_DEPTH, &callstack_hash);
  if (depth==0) return NULL;
  mi_callstack_t callstack = { depth, frames };
  const mi_thread_id_t thread_id = _mi_thread_id();
  mi_location_t* loc = mi_locations_find_or_insert(prof->profile_heap, &prof->locations, thread_id, &callstack, callstack_hash);
  return loc;
}

// ---------------------------------------------------------------------------
// Profiler callbacks and initialization
// ---------------------------------------------------------------------------

// Draws an integer approximating Exp(scale) to give a poisson distribution,
// instead of a fixed period, which avoids sampling bias from allocation
// patterns that happen to align with a constant period
static size_t mi_exp_sample(mi_theap_t* theap, size_t scale) {
  if (scale == 0) return 0;                                     // keep 0
  #if MI_PROFILE==2
  if (scale == 1) return 1;                                     // keep 1 as is (to allow sampling every allocation)
  #endif
  const size_t r  = _mi_theap_random_next(theap) | 1;
  const size_t lz = mi_clz(r);
  // with `r = 2^(MI_SIZE_BITS-1-lz) * m` (for `1 <= m < 2`), we have `-log2(U) = lz + 1 - log2(m)` (for `U = r/2^MI_SIZE_BITS`)
  const size_t frac_q8  = (lz >= MI_SIZE_BITS - 1 ? 0 : (r << (lz + 1)) >> (MI_SIZE_BITS - 8));  // `m-1` in Q8
  const size_t log2m_q8 = frac_q8 + ((frac_q8 * (256 - frac_q8) * 89) >> 16);  // `log2(1+x) ~ x + 0.347*x*(1-x)`
  const size_t bits_q8  = ((lz + 1) << 8) - log2m_q8;  // -log2(U) in Q8
  size_t ln_q8 = (bits_q8 * 45426) >> 16;             // -ln(U) = -log2(U)*ln(2)
  if (ln_q8 > (32 << 8)) { ln_q8 = (32 << 8); }       // clamp the (rare) long tail at 32 times the mean
  size_t next;
  if (mi_mul_overflow(scale >> 8, ln_q8, &next) || next > MI_SAMPLE_RATE_MAX) return MI_SAMPLE_RATE_MAX;
  return next + (((scale & 0xFF) * ln_q8) >> 8) + 1;  // `scale*ln_q8/256`; +1 avoids a zero threshold
}

// Check if the sampled bytes accumulated so far (batched per-thread) should trigger a shared
// countdown decrement, and if so, whether that decrement crosses zero (triggering a snapshot and
// resetting the countdown to `alloc_interval_size`).
static bool mi_interval_update(uint64_t ubytes_since_last_sample, size_t alloc_interval_size, _Atomic(mi_ssize_t)* alloc_interval_countdown, mi_ssize_t* interval_pending) {
  mi_ssize_t sampled_bytes = 0;
  mi_ssize_t bytes_since_last_sample= (ubytes_since_last_sample > MI_SSIZE_MAX ? MI_SSIZE_MAX : (mi_ssize_t)ubytes_since_last_sample);
  if (alloc_interval_size < MI_PROFILE_COUNTDOWN_MIN_INTERVAL || MI_SSIZE_MAX - *interval_pending < bytes_since_last_sample /* overflow? */) {
    // interval too small relative to the batching; always adjust
    sampled_bytes = bytes_since_last_sample;
  }
  else {
    // batch locally per-thread and only touch the shared (contended) countdown
    // once the local pending amount reaches `MI_PROFILE_COUNTDOWN_THRESHOLD`.
    *interval_pending += bytes_since_last_sample;
    if (*interval_pending >= (mi_ssize_t)MI_PROFILE_COUNTDOWN_THRESHOLD) {
      sampled_bytes = *interval_pending;
      *interval_pending = 0;
    }
  }
  // update profile interval countdown if we have sampled bytes
  if (sampled_bytes > 0) {
    const mi_ssize_t new_countdown = mi_atomic_addi_relaxed(alloc_interval_countdown, -sampled_bytes) - sampled_bytes;
    if (new_countdown <= 0 && new_countdown + sampled_bytes > 0) {
      mi_atomic_storess_relaxed(alloc_interval_countdown, (mi_ssize_t)alloc_interval_size);
      return true;
    }
  }
  return false;
}

// Check if the (global) in-use byte total has reached `inuse_interval_threshold`
static bool mi_inuse_threshold_update(size_t new_total, size_t inuse_interval_size, _Atomic(size_t)* inuse_interval_threshold) {
  size_t threshold = mi_atomic_load_relaxed(inuse_interval_threshold);
  while (new_total >= threshold) {
    size_t next_threshold = threshold + inuse_interval_size;
    while (next_threshold <= new_total) { next_threshold += inuse_interval_size; }  // skip ahead if we grew by more than one interval
    if (mi_atomic_cas_weak_relaxed(inuse_interval_threshold, &threshold, next_threshold)) {
      return true;
    }
    // CAS failed: `threshold` now holds the current value; retry (another thread may have already advanced it far enough)
  }
  return false;
}

// Check if `now` has reached `time_interval_deadline`, and if so, atomically advance the
// deadline to the next multiple of `time_interval_secs` beyond `now`, returning `true` (to the
// single thread that won the race) so a snapshot is triggered exactly once.
static bool mi_time_threshold_update(mi_msecs_t now, size_t time_interval_secs, _Atomic(mi_msecs_t)* time_interval_deadline) {
  const mi_msecs_t interval_msecs = (mi_msecs_t)time_interval_secs * 1000;
  mi_msecs_t deadline = mi_atomic_loadi64_relaxed(time_interval_deadline);
  while (now >= deadline) {
    mi_msecs_t next_deadline = deadline + interval_msecs;
    while (next_deadline <= now) { next_deadline += interval_msecs; }  // skip ahead if more than one interval has passed
    if (mi_atomic_casi64_strong_relaxed(time_interval_deadline, &deadline, next_deadline)) {
      return true;
    }
    // CAS failed: `deadline` now holds the current value; retry (another thread may have already advanced it far enough)
  }
  return false;
}

// Sample allocation event and update profiling data accordingly
static size_t mi_cdecl on_alloc(mi_profiler_t* profiler, mi_profiler_sample_data_t* data, void* ptr, size_t requested_size, size_t threshold, uint64_t bytes_since_last_sample, mi_heap_t* heap)
{
  MI_UNUSED(threshold); MI_UNUSED(ptr);
  mi_pprof_profiler_t* prof = downcast(profiler);
  mi_theap_t* theap = _mi_heap_theap(heap);
  mi_location_t* loc = mi_location_get(prof);  // nested allocations (e.g. by `backtrace`) are not sampled (see `mi_profiler_on_alloc`)
  const size_t new_threshold = mi_exp_sample(theap, prof->sample_threshold);  // randomized (Poisson) next sample threshold
  const size_t alloc_size = requested_size;            // or mi_heap_usable_size(heap,ptr) ?

  if (data!=NULL) {
    // pass the location to the corresponding mi_free (maybe called from another thread)
    mi_assert(data->user_data_size >= 2*sizeof(void*));
    data->user_data[0] = loc;
    data->user_data[1] = (void*)((uintptr_t)alloc_size);
  }
  if (loc!=NULL) {
    // `on_alloc` for a given location only ever runs on its one owning (allocating)
    // thread, so plain increments are safe here (no concurrent writers possible).
    loc->alloc_bytes += (int64_t)alloc_size;
    loc->alloc_count += 1;
    // `inuse_count`/`inuse_bytes` can be decremented concurrently by `on_free` on any
    // thread, so these updates must be atomic.
    mi_atomic_add_relaxed(&loc->inuse_bytes, alloc_size);
    mi_atomic_increment_relaxed(&loc->inuse_count);
  }

  if (prof->alloc_interval_size > 0) { // interval-based profiling enabled
    mi_ssize_t* pinterval_pending = (mi_ssize_t*)&theap->profiler_reserved1;
    if (mi_interval_update(bytes_since_last_sample, prof->alloc_interval_size, &prof->alloc_interval_countdown, pinterval_pending)) {
      mi_profiler_snapshot(profiler);
    }
  }
  if (prof->inuse_interval_size > 0) { // snapshot whenever (sampled) in-use memory has grown by `inuse_interval_size` bytes
    const size_t total_inuse_bytes = mi_atomic_add_relaxed(&prof->inuse_bytes, alloc_size) + alloc_size;
    if (mi_inuse_threshold_update(total_inuse_bytes, prof->inuse_interval_size, &prof->inuse_interval_threshold)) {
      mi_profiler_snapshot(profiler);
    }
  }
  if (prof->time_interval_secs > 0) { // snapshot whenever `time_interval_secs` seconds have passed
    if (mi_time_threshold_update(_mi_clock_now(), prof->time_interval_secs, &prof->time_interval_deadline)) {
      mi_profiler_snapshot(profiler);
    }
  }
  return new_threshold;
}

// Sample free event and update profiling data accordingly
static void mi_cdecl on_free(mi_profiler_t* profiler, mi_profiler_sample_data_t* data, void* ptr, mi_heap_t* heap)
{
  MI_UNUSED(heap); MI_UNUSED(ptr);
  mi_pprof_profiler_t* prof = downcast(profiler);
  if (data!=NULL) {
    mi_assert(data->user_data_size >= 2*sizeof(void*));
    mi_location_t* loc = (mi_location_t*)data->user_data[0];
    const size_t freed_bytes = (size_t)((uintptr_t)data->user_data[1]);
    if (loc!=NULL) {
      // different threads can free allocations that share the same (allocating-thread,
      // callstack) location concurrently, so these updates must be atomic.
      mi_atomic_sub_relaxed(&loc->inuse_bytes, freed_bytes);
      mi_atomic_decrement_relaxed(&loc->inuse_count);
    }
    if (prof->inuse_interval_size > 0) {  // keep the (global) in-use total in sync while interval-based snapshotting is enabled
      mi_atomic_sub_relaxed(&prof->inuse_bytes, freed_bytes);
    }
  }
}

static void mi_cdecl on_snapshot(mi_profiler_t* profiler) {
  mi_pprof_profiler_t* prof = downcast(profiler);
  _mi_pprof_profiler_snapshot(prof);
}

// Create a new profiler. If `base_file_name` ends with a recognized extension (`.heap` or `.text`),
// that extension selects the text snapshot format and is stripped from the stored base file name
// (the actual snapshot files always get their own `.<seq>.heap`/`.<seq>.pb` extension, see `_mi_pprof_profiler_snapshot`).
// Any other extension (or none) keeps the default (protobuf) snapshot format and is left untouched.
mi_profiler_t* mi_pprof_profiler_new(size_t initial_threshold, const char* base_file_name, size_t alloc_interval_size, size_t inuse_interval_size, size_t time_interval_secs) {
  // heap just for the profiler itself
  mi_heap_t* heap = mi_heap_new();
  if (heap == NULL) return NULL;
  mi_heap_profile_disable(heap);  // don't sample allocations in this heap

  // allocate and initialize the profiler structure from this heap
  mi_pprof_profiler_t* prof = mi_heap_zalloc_tp(mi_pprof_profiler_t,heap);
  if (prof == NULL) { mi_heap_delete(heap); return NULL; }
  prof->sample_threshold = initial_threshold;
  prof->profile_heap = heap;
  prof->base_file_name = (base_file_name != NULL ? mi_heap_strndup(heap, base_file_name, 1024) : NULL);
  prof->format_text = false;  // default: protobuf format, unless overridden by a recognized extension below
  if (prof->base_file_name != NULL) {
    // only look for a '.' extension after the last path separator (if any)
    const char* start = prof->base_file_name;
    const char* sep = _mi_strrchr(start, '/');
    if (sep != NULL) start = sep + 1;
    sep = _mi_strrchr(start, '\\');
    if (sep != NULL) start = sep + 1;
    const char* dot = _mi_strrchr(start, '.');
    if (dot != NULL) {
      if (_mi_streq(dot + 1, "heap") || _mi_streq(dot + 1, "text")) {
        prof->format_text = true;
      }
      prof->base_file_name[dot - prof->base_file_name] = 0;  // always strip the extension, whether recognized or not
    }
  }
  prof->alloc_interval_size = (alloc_interval_size > (size_t)MI_SSIZE_MAX ? (size_t)MI_SSIZE_MAX : alloc_interval_size);  // cap so it always fits in a `mi_ssize_t`
  mi_atomic_storess_relaxed(&prof->alloc_interval_countdown, (mi_ssize_t)prof->alloc_interval_size);  // 0 if disabled: `on_alloc` never decrements/checks in that case
  prof->inuse_interval_size = inuse_interval_size;
  mi_atomic_store_relaxed(&prof->inuse_interval_threshold, prof->inuse_interval_size);  // 0 if disabled (`inuse_interval_size` defaults to 0): `on_alloc` never checks in that case
  prof->time_interval_secs = time_interval_secs;
  prof->start_time_secs = _mi_pprof_time_secs();
  prof->start_clock = _mi_clock_now();
  mi_atomic_storei64_relaxed(&prof->time_interval_deadline, prof->start_clock + (mi_msecs_t)prof->time_interval_secs * 1000);  // deadline stays unreachable (now, i.e. always due) if disabled, but `on_alloc` never checks in that case
  if (!mi_locations_init(heap, &prof->locations)) {
    // `mi_heap_destroy` frees the live blocks of this private heap, i.e. the
    // profiler object and the base file name. (`mi_heap_delete` would instead
    // move them to the main heap, losing our only reference.)
    mi_heap_destroy(heap);
    return NULL;
  }
  prof->profiler.initial_sample_rate = initial_threshold;
  prof->profiler.sample_data_size = 2*sizeof(void*);  // we store the location and the allocation size in the sample data
  prof->profiler.on_alloc = &on_alloc;
  prof->profiler.on_free = &on_free;
  prof->profiler.on_snapshot = &on_snapshot;
  return &prof->profiler;
}

void mi_pprof_profiler_delete(mi_profiler_t* profiler) {
  if (profiler == NULL) return;
  mi_pprof_profiler_t* prof = downcast(profiler);
  mi_heap_t* heap = prof->profile_heap;
  mi_locations_done(heap, &prof->locations);
  mi_free(prof->base_file_name);
  mi_free(prof);
  mi_heap_delete(heap);
}

// ---------------------------------------------------------------------------
// A basic hash table of locations. All memory (the bucket array, the
// locations, and their callstack frames) is allocated from the profiler's
// own heap so it does not interfere with the profiled allocations.
// Collisions are resolved with simple chaining.
//
// `on_alloc` and `on_free` (and thus `mi_locations_find_or_insert`) can run
// concurrently on multiple threads and are made thread-safe without a lock:
// a bucket is a singly-linked list that only ever grows by prepending a new,
// fully initialized location; once published, a location's identifying
// fields (`hash`, `thread_id`, `callstack`) never change again. Moreover,
// since the hash (and thus the location itself) is derived from the calling
// thread's own id, a location is only ever looked up by the same thread
// that creates it, so there is never a concurrent insert race for the
// same location.
// ---------------------------------------------------------------------------

// A reasonably large prime bucket count to keep collision chains short
#define MI_LOCATIONS_BUCKET_COUNT  (16 * 1024 - 3)  /* 16381, prime */


// FNV-1a hash, mixed over the thread id and the (precomputed) callstack hash
static size_t mi_location_hash(mi_thread_id_t thread_id, size_t callstack_hash) {
  size_t hash = MI_FNV_OFFSET_BASIS;
  const size_t prime = MI_FNV_PRIME;
  hash = (hash ^ (size_t)thread_id) * prime;
  hash = (hash ^ callstack_hash) * prime;
  return hash;
}

// Initialize a location hash table
static bool mi_locations_init(mi_heap_t* heap, mi_locations_t* locations) {
  locations->bucket_count = MI_LOCATIONS_BUCKET_COUNT;
  locations->buckets = (_Atomic(mi_location_t*)*)mi_heap_zalloc(heap, locations->bucket_count * sizeof(*locations->buckets));
  mi_atomic_store_relaxed(&locations->count, (size_t)0);
  return (locations->buckets != NULL);
}

// Free all memory associated with the location hash table.
// Not thread-safe: assumes no concurrent `on_alloc`/`on_free` (see the note above).
static void mi_locations_done(mi_heap_t* heap, mi_locations_t* locations) {
  MI_UNUSED(heap);
  if (locations->buckets == NULL) return;
  for (size_t i = 0; i < locations->bucket_count; i++) {
    mi_location_t* loc = mi_atomic_load_ptr_relaxed(mi_location_t, &locations->buckets[i]);
    while (loc != NULL) {
      mi_location_t* next = mi_atomic_load_ptr_relaxed(mi_location_t, &loc->next);
      if (loc->callstack.frames != NULL) {
        mi_free(loc->callstack.frames);
      }
      mi_free(loc);
      loc = next;
    }
  }
  mi_free(locations->buckets);
  locations->buckets = NULL;
  mi_atomic_store_relaxed(&locations->count, (size_t)0);
}

// Match a location's identifying fields; these never change after publication.
static bool mi_location_matches(const mi_location_t* loc, size_t hash, mi_thread_id_t thread_id, const mi_callstack_t* callstack) {
  return (loc->hash == hash && loc->thread_id == thread_id &&
          loc->callstack.count == callstack->count &&
          (callstack->count == 0 || mi_memcmp(loc->callstack.frames, callstack->frames, callstack->count * sizeof(void*)) == 0));
}

// Find an existing location matching `thread_id` and `callstack`, or insert
// a fresh, zero-initialized one (with `hash` and identifying fields filled in)
// if none exists yet. Returns NULL only on allocation failure.
// Thread-safe (see the note above): the common case (an existing location)
// is a lock-free, read-only traversal using relaxed loads; only inserting a
// genuinely new location uses a CAS (and even then, only ever races against
// other threads inserting *different* locations into the same bucket).
static mi_location_t* mi_locations_find_or_insert(mi_heap_t* heap, mi_locations_t* locations, mi_thread_id_t thread_id, const mi_callstack_t* callstack, size_t callstack_hash) {
  if (locations->buckets == NULL) return NULL;
  const size_t hash = mi_location_hash(thread_id, callstack_hash);
  const size_t idx = hash % locations->bucket_count;
  _Atomic(mi_location_t*)* bucket = &locations->buckets[idx];

  // fast path: lock-free, read-only lookup in the (possibly concurrently growing) chain
  for (mi_location_t* loc = mi_atomic_load_ptr_relaxed(mi_location_t, bucket); loc != NULL;
       loc = mi_atomic_load_ptr_relaxed(mi_location_t, &loc->next))
  {
    if (mi_location_matches(loc, hash, thread_id, callstack)) return loc;
  }

  // not found: allocate and fully initialize a new location; it is not yet
  // visible to any other thread so plain (non-atomic) stores are fine here.
  mi_location_t* loc = mi_heap_zalloc_tp(mi_location_t,heap);
  if (loc == NULL) return NULL;
  if (callstack->count > 0) {
    loc->callstack.frames = (void**)mi_heap_mallocn(heap, callstack->count, sizeof(void*));
    if (loc->callstack.frames == NULL) { mi_free(loc); return NULL; }  // free the not-yet-published location
    mi_memcpy(loc->callstack.frames, callstack->frames, callstack->count * sizeof(void*));
  }
  loc->callstack.count = callstack->count;
  loc->thread_id = thread_id;
  loc->hash = hash;

  // publish the new location at the head of the bucket with a CAS loop. Since
  // a location's key includes the calling thread's own id, no other thread
  // can ever race to insert this *same* key -- a CAS can only fail because
  // some other thread prepended an unrelated location to this bucket in the
  // meantime, so we simply retry with the updated head (no duplicate check
  // needed, unlike a typical lock-free insert).
  mi_location_t* head = mi_atomic_load_ptr_relaxed(mi_location_t, bucket);
  do {
    mi_atomic_store_ptr_relaxed(mi_location_t, &loc->next, head);  // `loc` not yet published: plain store is fine
  } while (!mi_atomic_cas_ptr_weak_release(mi_location_t, bucket, &head, loc));  // `head` is updated to the current value on failure
  mi_atomic_increment_relaxed(&locations->count);  // just a statistic
  return loc;
}
