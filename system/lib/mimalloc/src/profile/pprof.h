/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#pragma once
#ifndef MI_PPROF_H
#define MI_PPROF_H

// Shared by `pprof.c`, `pprof-os.c`, and `pprof-write.c`

#include "mimalloc.h"
#include "mimalloc/profile/pprof.h"

// Use the builtin profiler with OS specific backtraces and snapshot files
#ifndef MI_PROFILE_USE_BUILTIN
#define MI_PROFILE_USE_BUILTIN  (MI_PROFILE)
#endif

typedef struct mi_location_s  mi_location_t;
typedef struct mi_locations_s mi_locations_t;
typedef struct mi_callstack_s mi_callstack_t;

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------
#define MI_MAX_BACKTRACE_DEPTH    (100)
#define MI_BACKTRACE_SKIP_FRAMES  (3)

// Threshold (in bytes) for the thread-local interval countdown batching in `on_alloc`
#ifndef MI_PROFILE_COUNTDOWN_THRESHOLD
#define MI_PROFILE_COUNTDOWN_THRESHOLD        (8*MI_KiB)
#endif
#define MI_PROFILE_COUNTDOWN_MIN_INTERVAL     (8*MI_PROFILE_COUNTDOWN_THRESHOLD)

// A callstack represents a sequence of function call frames captured at a specific point in time.
struct mi_callstack_s {
  size_t  count;
  void**  frames;
};

// A location is a unique combination of a thread and runtime callstack for
// which we track allocation and deallocation statistics.
//
// Locations are only ever inserted, or mutated in-place after publication, and a location is only ever looked up using its own
// `hash`/`thread_id`/`callstack` which allows a lock-free lookup of locations in the hash table.
struct mi_location_s {
  _Atomic(struct mi_location_s*) next;  // next location in the same hash bucket (for chaining on collision);
  mi_thread_id_t     thread_id;
  mi_callstack_t    callstack;
  size_t            hash;         // hash of the thread_id and callstack
  int64_t           alloc_count;  // only ever written by the single (allocating) owning thread: not atomic
  int64_t           alloc_bytes;
  _Atomic(size_t)   inuse_count;  // currently in-use (allocated but not yet freed) objects/bytes for this location;
  _Atomic(size_t)   inuse_bytes;  // incremented in `on_alloc` and decremented in `on_free`, both possibly on any thread
};

// A basic hash table of locations. The bucket array is fixed-size (allocated
// once in `mi_locations_init`) and each bucket is a lock-free singly-linked
// list that only ever grows by prepending (see `mi_locations_find_or_insert`).
struct mi_locations_s {
  size_t                      bucket_count;  // number of buckets (fixed size)
  _Atomic(mi_location_t*)*    buckets;       // array of `bucket_count` chain heads
  _Atomic(size_t)             count;         // number of locations currently stored (just a statistic)
};

// Our profiler.
typedef struct {
  mi_profiler_t        profiler;                  // mimalloc profiler info
  mi_heap_t*           profile_heap;              // heap used for all profiler allocations (so these don't interfere)
  mi_locations_t       locations;                 // hash table of call locations
  size_t               sample_threshold;          // current sample threshold
  size_t               snapshot_count;                // number of times `mi_profiler_snapshot` was called (used to number the snapshot files)
  char*                base_file_name;            // base file name for snapshot files, e.g. "<base_file_name>.<seq>.heap"
  bool                 format_text;               // text or protobuf snapshot format
  size_t               alloc_interval_size;       // if >0, automatically snapshot every `alloc_interval_size` allocated bytes (capped at `MI_SSIZE_MAX`)
  _Atomic(mi_ssize_t)  alloc_interval_countdown;  // bytes remaining until the next automatic snapshot (can go negative; decremented concurrently by `on_alloc` on any thread)
  size_t               inuse_interval_size;       // if >0, automatically snapshot every time (sampled) in-use bytes grow by `inuse_interval_size` bytes
  _Atomic(size_t)      inuse_bytes;               // running total of (sampled) in-use bytes across all locations (can never exceed the address space); incremented in `on_alloc`, decremented in `on_free`, both possibly on any thread
  _Atomic(size_t)      inuse_interval_threshold;  // next `inuse_bytes` total that triggers a snapshot; advanced (monotonically increasing) whenever reached in `on_alloc`
  size_t               time_interval_secs;        // if >0, automatically snapshot every `time_interval_secs` seconds (checked from `on_alloc`)
  _Atomic(mi_msecs_t)  time_interval_deadline;    // next `_mi_clock_now()` value that triggers a snapshot; advanced (monotonically increasing) whenever reached in `on_alloc`
  int64_t              start_time_secs;           // wall-clock time at creation (seconds since the Unix epoch)
  mi_msecs_t           start_clock;               // `_mi_clock_now()` at creation (monotonic, so snapshot times are `start_time_secs` + elapsed clock)
} mi_pprof_profiler_t;

// FNV-1a hashing (of locations and backtraces)
#if SIZE_MAX > UINT32_MAX
#define MI_FNV_OFFSET_BASIS  ((size_t)0xcbf29ce484222325ULL)  // FNV-1a 64-bit offset basis
#define MI_FNV_PRIME         ((size_t)0x100000001b3ULL)       // FNV-1a 64-bit prime
#else
#define MI_FNV_OFFSET_BASIS  ((size_t)0x811c9dc5UL)           // FNV-1a 32-bit offset basis
#define MI_FNV_PRIME         ((size_t)0x01000193UL)           // FNV-1a 32-bit prime
#endif

// defined in `pprof-os.c`
size_t  _mi_pprof_backtrace(void** buffer, size_t max_depth, size_t* hash);  // `buffer` must have room for `max_depth + MI_BACKTRACE_SKIP_FRAMES + 1` entries (see definition)
int64_t _mi_pprof_time_secs(void);  // wall-clock seconds since the Unix epoch (0 if unavailable)

// defined in `pprof-write.c`
void    _mi_pprof_profiler_snapshot(mi_pprof_profiler_t* prof);


#if MI_PROFILE_USE_BUILTIN

// A loaded module's address range and (if known) its file path; used both to
// print the `MAPPED_LIBRARIES` trailer of the text format and to build the
// `Mapping` entries of the protobuf format (see `mi_pprof_snapshot_proto`).
typedef struct {
  uintptr_t start;
  uintptr_t end;
  uint64_t  file_offset;
  char*     path;  // heap-allocated (owned), or NULL if unknown
} mi_pprof_module_t;

typedef struct {
  mi_pprof_module_t* modules;
  size_t              count;
  size_t              capacity;
  bool                failed;  // set if an allocation failed; the mapping list is then incomplete
} mi_pprof_modules_t;

// defined in `pprof-os.c`
typedef struct mi_pprof_file_s mi_pprof_file_t;   // buffered snapshot file (does not use stdio)
mi_pprof_file_t* _mi_pprof_fopen(mi_heap_t* heap, const char* fname, bool binary);
void    _mi_pprof_fwrite(mi_pprof_file_t* f, const void* data, size_t size);
void    _mi_pprof_fputs(mi_pprof_file_t* f, const char* s);
void    _mi_pprof_fprintf(mi_pprof_file_t* f, const char* fmt, ...);
bool    _mi_pprof_fclose(mi_pprof_file_t* f);  // returns `false` if any write failed
void    _mi_pprof_collect_modules(mi_heap_t* heap, mi_pprof_modules_t* mods);
void    _mi_pprof_modules_done(mi_pprof_modules_t* mods);
void    _mi_pprof_write_mapped_libraries(mi_pprof_file_t* f);
#endif

#endif
