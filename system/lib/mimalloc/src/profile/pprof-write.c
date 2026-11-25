/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
Disclaimer: Unlike the rest of the considerate hand-crafted mimalloc sources,
this file was written with AI assistance (for the protobuf format generation).
-----------------------------------------------------------------------------*/

// Write snapshots of the pprof profiler in the (textual) pprof heap profile format or the protobuf format.

#include "mimalloc.h"
#include "mimalloc/util/libc.h"
#include "mimalloc/util/options.h"
#include "mimalloc/util/stats.h"
#include "pprof.h"

#if !MI_PROFILE_USE_BUILTIN

void _mi_pprof_profiler_snapshot(mi_pprof_profiler_t* profiler) {
  MI_UNUSED(profiler);
}

#else

// forward declarations; implemented further below.
static bool mi_pprof_snapshot_text(mi_pprof_profiler_t* prof, const char* fname);
static bool mi_pprof_snapshot_proto(mi_pprof_profiler_t* prof, const char* fname);

// Write out the current profiler data to a new snapshot file named
// `<base_file_name>.<seq>.<ext>` with an incrementing sequence number
// (`.heap` for the text format, mimicking the naming used by the original
// (gperftools) pprof heap profiler; `.pb` for the (uncompressed) protobuf
// format). The format is chosen when the profiler is created (see
// `mi_pprof_profiler_new`). The base file name is also set at creation time;
// if it is NULL, no snapshot is written and this function does nothing.
void _mi_pprof_profiler_snapshot(mi_pprof_profiler_t* prof) {
  if (prof == NULL) return;
  // note: we do not stop the profiler while writing as a concurrent `mi_profiler_stop` by the user could then be undone;
  // the location table is lock-free and only read here, and a nested (interval) snapshot is skipped by the snapshot guard.
  mi_locations_t* locations = &prof->locations;
  if (locations->buckets != NULL && prof->base_file_name != NULL) {
    const size_t seq = ++prof->snapshot_count;
    char fname[1024];
    bool ok;
    if (!prof->format_text) {
      _mi_snprintf(fname, sizeof(fname), "%s.%04zu.pb", prof->base_file_name, seq);
      ok = mi_pprof_snapshot_proto(prof, fname);
    }
    else {
      _mi_snprintf(fname, sizeof(fname), "%s.%04zu.heap", prof->base_file_name, seq);
      ok = mi_pprof_snapshot_text(prof, fname);
    }
    // A snapshot is best effort: it can fail when the profiler's own heap is out
    // of memory (or the file cannot be created), in which case no (or only a
    // partial) snapshot is written. Never let that affect the profiled program.
    if (ok) {
      _mi_verbose_message("pprof profiler snapshot: %s\n", fname);
    }
    else {
      _mi_verbose_message("pprof profiler: unable to write snapshot (out of memory?)\n");
    }
  }
}

// ---------------------------------------------------------------------------
// Estimate the actual values from the sampled ones (using Q32 fixed-point arithmetic)
// ---------------------------------------------------------------------------

#define MI_Q32_ONE  ((uint64_t)1 << 32)

// `n/d` in Q32 (saturating) without needing 128-bit arithmetic
static uint64_t mi_div_q32(uint64_t n, uint64_t d) {
  uint64_t q = n / d;
  if (q >= MI_Q32_ONE) return UINT64_MAX;
  uint64_t r = n % d;
  for (int i = 0; i < 32; i++) {  // long division for the 32 fractional bits (where `r < d`)
    q <<= 1;
    if (r >= d - r) { r -= d - r; q |= 1; } else { r <<= 1; }
  }
  return q;
}

// `1 - e^(-x)` in Q32 for `x` in Q32: halve `x` until small, use a Taylor series, and double back using `1 - e^(-2x) = y*(2-y)` where `y = 1 - e^(-x)`
static uint64_t mi_one_minus_exp_neg_q32(uint64_t x) {
  int k = 0;
  while (x > MI_Q32_ONE/2) { x >>= 1; k++; }
  uint64_t y = x;
  uint64_t term = x;
  for (unsigned i = 2; i <= 10; i++) {  // partial sums stay positive as the terms decrease
    term = ((term * x) >> 32) / i;
    y = ((i & 1) != 0 ? y + term : y - term);
  }
  while (k-- > 0) { y = (y * (2*MI_Q32_ONE - y)) >> 32; }  // no overflow as `y < 1`
  return y;
}

// `n/p` for a probability `p` in Q32
static int64_t mi_unsample_value(uint64_t n, uint64_t p) {
  const uint64_t m = mi_div_q32(n, p);
  return (m > INT64_MAX ? INT64_MAX : (int64_t)m);
}

// Estimate the actual count and bytes from the sampled ones (as pprof does for sampled heap profiles):
// an object of size `s` is sampled with probability `1 - e^(-s/rate)`.
static void mi_pprof_unsample(int64_t* count, int64_t* bytes, size_t rate) {
  if (*count <= 0 || *bytes <= 0 || rate <= 1) return;
  const uint64_t n = (uint64_t)*count;
  const uint64_t b = (uint64_t)*bytes;
  const uint64_t avg = b / n;
  if (avg/32 >= rate) return;  // sampled with probability 1 (within 1e-13)
  const uint64_t x = mi_div_q32(avg, rate) + mi_div_q32(b % n, n) / rate;  // `(b/n)/rate` in Q32
  uint64_t p = mi_one_minus_exp_neg_q32(x);
  if (p == 0) { p = 1; }
  *count = mi_unsample_value(n, p);
  *bytes = mi_unsample_value(b, p);
}

// ---------------------------------------------------------------------------
// Writing the profile in the (original, textual) pprof heap profile format
// ---------------------------------------------------------------------------

// Write out the current profiler data in the pprof heap profile text format:
// a header line with the totals, followed by one line per unique call
// location, and a trailer with the mapped libraries (used by `pprof` to
// symbolize the addresses).
static bool mi_pprof_snapshot_text(mi_pprof_profiler_t* prof, const char* fname) {
  mi_locations_t* locations = &prof->locations;
  mi_pprof_file_t* f = _mi_pprof_fopen(prof->profile_heap, fname, false);
  if (f == NULL) return false;

  // first pass: compute the totals over all locations
  // (sampling may continue concurrently so the totals are only approximately consistent;
  // `inuse_count`/`inuse_bytes` are `_Atomic` fields so we use a relaxed load for those)
  uint64_t total_inuse_objects = 0;
  uint64_t total_inuse_bytes   = 0;
  uint64_t total_alloc_objects = 0;
  uint64_t total_alloc_bytes   = 0;
  for (size_t i = 0; i < locations->bucket_count; i++) {
    for (mi_location_t* loc = mi_atomic_load_ptr_relaxed(mi_location_t, &locations->buckets[i]); loc != NULL; loc = mi_atomic_load_ptr_relaxed(mi_location_t, &loc->next)) {
      total_inuse_objects += (uint64_t)mi_atomic_load_relaxed(&loc->inuse_count);
      total_inuse_bytes   += (uint64_t)mi_atomic_load_relaxed(&loc->inuse_bytes);
      total_alloc_objects += (uint64_t)loc->alloc_count;
      total_alloc_bytes   += (uint64_t)loc->alloc_bytes;
    }
  }

  // the sample rate in the header lets pprof estimate the actual counts and bytes
  _mi_pprof_fprintf(f, "heap profile: %6l8d: %8l8d [%6l8d: %8l8d] @ heap_v2/%zu\n",
             total_inuse_objects, total_inuse_bytes, total_alloc_objects, total_alloc_bytes, prof->sample_threshold);

  // second pass: write one line per unique call location
  for (size_t i = 0; i < locations->bucket_count; i++) {
    for (mi_location_t* loc = mi_atomic_load_ptr_relaxed(mi_location_t, &locations->buckets[i]); loc != NULL; loc = mi_atomic_load_ptr_relaxed(mi_location_t, &loc->next)) {
      const size_t inuse_count = mi_atomic_load_relaxed(&loc->inuse_count);
      const size_t inuse_bytes = mi_atomic_load_relaxed(&loc->inuse_bytes);
      _mi_pprof_fprintf(f, "%6l8d: %8l8d [%6l8d: %8l8d] @",
                 (uint64_t)inuse_count, (uint64_t)inuse_bytes,
                 (uint64_t)loc->alloc_count, (uint64_t)loc->alloc_bytes);
      for (size_t j = 0; j < loc->callstack.count; j++) {
        _mi_pprof_fprintf(f, " 0x%tx", (size_t)(uintptr_t)loc->callstack.frames[j]);
      }
      _mi_pprof_fputs(f, "\n");
    }
  }

  _mi_pprof_write_mapped_libraries(f);
  return _mi_pprof_fclose(f);
}


// ---------------------------------------------------------------------------
// Writing the profile in the modern `pprof` protobuf format
// (`perftools.profiles.Profile`, see
//  https://github.com/google/pprof/blob/main/proto/profile.proto)
//
// On-disk, `pprof` profiles are normally gzip-compressed, but `pprof` also
// accepts a *raw* (uncompressed) serialized `Profile` message: it only tries
// to gunzip the input if it starts with the gzip magic bytes, and otherwise
// parses it directly as a protobuf message. We take advantage of that so we
// don't need to implement a deflate/gzip writer.
//
// We still need the locations hash table for this format: samples are
// aggregated per call site (thread + callstack), exactly as for the text
// format, rather than emitting one `Sample` per individual allocation. What
// changes is the *representation* of a callstack: instead of a flat list of
// hex addresses embedded directly in a text line, each frame address becomes
// its own (deduplicated) `Location` entry, referenced by id from `Sample.
// location_id`. The originating thread id -- which the text format doesn't
// record explicitly (it is only implicit in which addresses got sampled) --
// is attached to each `Sample` via a numeric `Label` (pprof has no dedicated
// thread-id field, but labels are the generic mechanism for this).
// ---------------------------------------------------------------------------

// A simple growable byte buffer, allocated from the profiler's own heap.
// If an allocation fails, `failed` is set and all further writes become no-ops;
// the caller must check it and discard the whole snapshot.
typedef struct {
  mi_heap_t* heap;
  uint8_t*   data;
  size_t     size;
  size_t     capacity;
  bool       failed;
} mi_pb_buf_t;

static void mi_pb_init(mi_pb_buf_t* b, mi_heap_t* heap) {
  b->heap = heap;
  b->data = NULL;
  b->size = 0;
  b->capacity = 0;
  b->failed = false;
}

static void mi_pb_done(mi_pb_buf_t* b) {
  mi_free(b->data);
  b->data = NULL;
  b->size = 0;
  b->capacity = 0;
  b->failed = false;
}

static bool mi_pb_reserve(mi_pb_buf_t* b, size_t extra) {
  if (b->failed) return false;
  if (b->size + extra <= b->capacity) return true;
  size_t new_capacity = (b->capacity == 0 ? 4096 : b->capacity * 2);
  while (new_capacity < b->size + extra) { new_capacity *= 2; }
  uint8_t* const new_data = (uint8_t*)mi_heap_realloc(b->heap, b->data, new_capacity);
  if (new_data == NULL) { b->failed = true; return false; }  // keep the old buffer (and capacity) intact
  b->data = new_data;
  b->capacity = new_capacity;
  return true;
}

static void mi_pb_bytes(mi_pb_buf_t* b, const void* p, size_t n) {
  if (n == 0) return;  // avoid calling memcpy with a NULL source
  if (!mi_pb_reserve(b, n)) return;
  mi_memcpy(b->data + b->size, p, n);
  b->size += n;
}

static void mi_pb_byte(mi_pb_buf_t* b, uint8_t byte) {
  mi_pb_bytes(b, &byte, 1);
}

// Unsigned LEB128 (protobuf "varint") encoding.
static void mi_pb_varint(mi_pb_buf_t* b, uint64_t v) {
  do {
    uint8_t byte = (uint8_t)(v & 0x7F);
    v >>= 7;
    if (v != 0) { byte |= 0x80; }
    mi_pb_byte(b, byte);
  } while (v != 0);
}

static void mi_pb_tag(mi_pb_buf_t* b, uint32_t field, uint32_t wiretype) {
  mi_pb_varint(b, ((uint64_t)field << 3) | wiretype);
}

// Scalar varint fields; proto3 encoders may omit fields set to their default
// (0/false) value, which we do here to keep the output compact.
static void mi_pb_uint64_field(mi_pb_buf_t* b, uint32_t field, uint64_t v) {
  if (v == 0) return;
  mi_pb_tag(b, field, 0);
  mi_pb_varint(b, v);
}

static void mi_pb_int64_field(mi_pb_buf_t* b, uint32_t field, int64_t v) {
  if (v == 0) return;
  mi_pb_tag(b, field, 0);
  mi_pb_varint(b, (uint64_t)v);  // proto3 plain int64 is encoded as a varint of its 2's complement bit pattern
}

// Length-delimited fields (string/bytes/embedded message) are always emitted,
// even when empty: this matters for `string_table[0]` (which must be the
// empty string) and for repeated message fields (an empty message is still a
// valid, distinct list entry).
static void mi_pb_len_field(mi_pb_buf_t* b, uint32_t field, const void* data, size_t len) {
  mi_pb_tag(b, field, 2);
  mi_pb_varint(b, len);
  mi_pb_bytes(b, data, len);
}

static void mi_pb_message_field(mi_pb_buf_t* b, uint32_t field, const mi_pb_buf_t* msg) {
  if (msg->failed) { b->failed = true; return; }  // propagate a failure from the nested message
  mi_pb_len_field(b, field, msg->data, msg->size);
}

// A packed repeated scalar field (used for `Sample.location_id`/`Sample.value`):
// all elements are varint-encoded and concatenated into one length-delimited blob.
static void mi_pb_packed_uint64_field(mi_pb_buf_t* b, uint32_t field, const uint64_t* values, size_t count) {
  if (count == 0) return;
  mi_pb_buf_t tmp;
  mi_pb_init(&tmp, b->heap);
  for (size_t i = 0; i < count; i++) { mi_pb_varint(&tmp, values[i]); }
  mi_pb_message_field(b, field, &tmp);
  mi_pb_done(&tmp);
}

static void mi_pb_packed_int64_field(mi_pb_buf_t* b, uint32_t field, const int64_t* values, size_t count) {
  mi_pb_packed_uint64_field(b, field, (const uint64_t*)values, count);  // same varint bit pattern
}

// ---------------------------------------------------------------------------
// String table: dedups strings into `Profile.string_table` (field 6) and
// hands back the (1-based; index 0 is reserved for "") index of each string.
// A linear search is fine here: a profile typically only has a handful to a
// few hundred distinct strings (sample type names, mapped file paths).
// ---------------------------------------------------------------------------

typedef struct {
  char*   str;
  int64_t index;
} mi_pb_string_entry_t;

typedef struct {
  mi_heap_t*            heap;
  mi_pb_string_entry_t* entries;
  size_t                count;
  size_t                capacity;
  bool                  failed;  // set if an allocation failed; the table is then incomplete
  mi_pb_buf_t           table;  // the serialized `string_table` entries (Profile field 6)
} mi_pb_strings_t;

static void mi_pb_strings_init(mi_pb_strings_t* strings, mi_heap_t* heap) {
  strings->heap = heap;
  strings->entries = NULL;
  strings->count = 0;
  strings->capacity = 0;
  strings->failed = false;
  mi_pb_init(&strings->table, heap);
  mi_pb_len_field(&strings->table, 6, "", 0);  // index 0 must always be ""
}

static void mi_pb_strings_done(mi_pb_strings_t* strings) {
  for (size_t i = 0; i < strings->count; i++) { mi_free(strings->entries[i].str); }
  mi_free(strings->entries);
  mi_pb_done(&strings->table);
}

static int64_t mi_pb_strings_intern(mi_pb_strings_t* strings, const char* s) {
  if (strings->failed) return 0;
  if (s == NULL) { s = ""; }
  for (size_t i = 0; i < strings->count; i++) {
    if (_mi_streq(strings->entries[i].str, s)) { return strings->entries[i].index; }
  }
  if (strings->count == strings->capacity) {
    const size_t new_capacity = (strings->capacity == 0 ? 32 : strings->capacity * 2);
    mi_pb_string_entry_t* const new_entries = (mi_pb_string_entry_t*)mi_heap_realloc(strings->heap, strings->entries, new_capacity * sizeof(*strings->entries));
    if (new_entries == NULL) { strings->failed = true; return 0; }
    strings->entries = new_entries;
    strings->capacity = new_capacity;
  }
  char* const copy = mi_heap_strndup(strings->heap, s, 1024);
  if (copy == NULL) { strings->failed = true; return 0; }
  const int64_t index = (int64_t)(strings->count + 1);  // index 0 is reserved for ""
  strings->entries[strings->count].str = copy;
  strings->entries[strings->count].index = index;
  strings->count++;
  mi_pb_len_field(&strings->table, 6, s, _mi_strlen(s));
  if (strings->table.failed) { strings->failed = true; return 0; }
  return index;
}

// ---------------------------------------------------------------------------
// A small single-threaded hash map from a raw frame address to the id of its
// (deduplicated) `Location` entry. Only ever built and consulted from within
// `mi_pprof_snapshot_proto`, so (unlike the locations hash table) it needs no
// atomics at all.
// ---------------------------------------------------------------------------

typedef struct mi_pb_addr_entry_s {
  struct mi_pb_addr_entry_s* next;
  uintptr_t                  addr;
  uint64_t                   id;
} mi_pb_addr_entry_t;

typedef struct {
  mi_heap_t*            heap;
  size_t                bucket_count;
  mi_pb_addr_entry_t**  buckets;
  bool                  failed;  // set if an allocation failed (the map is then unusable)
} mi_pb_addr_map_t;

static void mi_pb_addr_map_init(mi_pb_addr_map_t* map, mi_heap_t* heap, size_t bucket_count) {
  map->heap = heap;
  map->bucket_count = (bucket_count == 0 ? 1 : bucket_count);
  map->buckets = (mi_pb_addr_entry_t**)mi_heap_zalloc(heap, map->bucket_count * sizeof(*map->buckets));
  map->failed = (map->buckets == NULL);
}

static void mi_pb_addr_map_done(mi_pb_addr_map_t* map) {
  if (map->buckets == NULL) return;
  for (size_t i = 0; i < map->bucket_count; i++) {
    mi_pb_addr_entry_t* e = map->buckets[i];
    while (e != NULL) {
      mi_pb_addr_entry_t* const next = e->next;
      mi_free(e);
      e = next;
    }
  }
  mi_free(map->buckets);
  map->buckets = NULL;
}

// Returns the existing location id for `addr`, or assigns it `next_id` and
// returns that (setting `*is_new` to true) if this is the first time `addr`
// is seen.
static uint64_t mi_pb_addr_map_find_or_insert(mi_pb_addr_map_t* map, mi_addr_t addr, uint64_t next_id, bool* is_new) {
  if (map->failed || map->buckets == NULL) { map->failed = true; *is_new = false; return 0; }
  const size_t idx = (size_t)(addr ^ (addr >> 16)) % map->bucket_count;
  for (mi_pb_addr_entry_t* e = map->buckets[idx]; e != NULL; e = e->next) {
    if (e->addr == addr) { *is_new = false; return e->id; }
  }
  mi_pb_addr_entry_t* e = mi_heap_malloc_tp(mi_pb_addr_entry_t, map->heap);
  if (e == NULL) { map->failed = true; *is_new = false; return 0; }
  e->addr = addr;
  e->id = next_id;
  e->next = map->buckets[idx];
  map->buckets[idx] = e;
  *is_new = true;
  return next_id;
}

// Find the (1-based) `Mapping` id of the module containing `addr`, or 0 if
// `addr` doesn't fall within any collected module range.
static uint64_t mi_pprof_find_mapping_id(const mi_pprof_modules_t* mods, uintptr_t addr) {
  for (size_t i = 0; i < mods->count; i++) {
    if (addr >= mods->modules[i].start && addr < mods->modules[i].end) { return (uint64_t)(i + 1); }
  }
  return 0;
}

// Build the current profiler data as a raw (uncompressed) `perftools.profiles.Profile`
// protobuf message in `profile` (see the file-level comment above).
// Returns `false` if any allocation failed; the caller releases all buffers in either case.
static bool mi_pprof_build_proto(mi_pprof_profiler_t* prof, mi_pb_buf_t* profile, mi_pb_strings_t* strings,
                                 mi_pprof_modules_t* mods, mi_pb_addr_map_t* addr_map)
{
  mi_heap_t* heap = prof->profile_heap;
  mi_locations_t* locations = &prof->locations;

  const int64_t str_count         = mi_pb_strings_intern(strings, "count");
  const int64_t str_bytes         = mi_pb_strings_intern(strings, "bytes");
  const int64_t str_alloc_objects = mi_pb_strings_intern(strings, "alloc_objects");
  const int64_t str_alloc_bytes   = mi_pb_strings_intern(strings, "alloc_bytes");
  const int64_t str_inuse_objects = mi_pb_strings_intern(strings, "inuse_objects");
  const int64_t str_inuse_bytes   = mi_pb_strings_intern(strings, "inuse_bytes");
  const int64_t str_space         = mi_pb_strings_intern(strings, "space");
  const int64_t str_thread        = mi_pb_strings_intern(strings, "thread");
  if (strings->failed) return false;

  // Profile.sample_type = 1: [alloc_objects:count, alloc_bytes:bytes, inuse_objects:count, inuse_bytes:bytes]
  {
    const int64_t types[4] = { str_alloc_objects, str_alloc_bytes, str_inuse_objects, str_inuse_bytes };
    const int64_t units[4] = { str_count,         str_bytes,       str_count,         str_bytes };
    for (size_t i = 0; i < 4; i++) {
      mi_pb_buf_t vt;
      mi_pb_init(&vt, heap);
      mi_pb_int64_field(&vt, 1, types[i]);
      mi_pb_int64_field(&vt, 2, units[i]);
      mi_pb_message_field(profile, 1, &vt);
      mi_pb_done(&vt);
    }
  }

  // Profile.mapping = 3: the modules loaded into this process, so `pprof` can
  // symbolize the (raw) addresses recorded in each `Location` below.
  _mi_pprof_collect_modules(heap, mods);
  if (mods->failed) return false;
  for (size_t i = 0; i < mods->count; i++) {
    const mi_pprof_module_t* m = &mods->modules[i];
    mi_pb_buf_t mapping;
    mi_pb_init(&mapping, heap);
    mi_pb_uint64_field(&mapping, 1, (uint64_t)(i + 1));    // id
    mi_pb_uint64_field(&mapping, 2, (uint64_t)m->start);   // memory_start
    mi_pb_uint64_field(&mapping, 3, (uint64_t)m->end);     // memory_limit
    mi_pb_uint64_field(&mapping, 4, m->file_offset);       // file_offset
    if (m->path != NULL) {
      mi_pb_int64_field(&mapping, 5, mi_pb_strings_intern(strings, m->path));  // filename
    }
    mi_pb_message_field(profile, 3, &mapping);
    mi_pb_done(&mapping);
    if (profile->failed || strings->failed) return false;
  }

  // Profile.location = 4 and Profile.sample = 2: one (deduplicated) `Location`
  // per unique frame address, and one `Sample` per unique call site (i.e. one
  // per entry in our own locations hash table), exactly mirroring the text
  // format's aggregation.
  mi_pb_addr_map_init(addr_map, heap, locations->bucket_count);
  if (addr_map->failed) return false;
  uint64_t next_location_id = 1;

  for (size_t i = 0; i < locations->bucket_count; i++) {
    for (mi_location_t* loc = mi_atomic_load_ptr_relaxed(mi_location_t, &locations->buckets[i]); loc != NULL; loc = mi_atomic_load_ptr_relaxed(mi_location_t, &loc->next)) {
      const size_t inuse_count = mi_atomic_load_relaxed(&loc->inuse_count);
      const size_t inuse_bytes = mi_atomic_load_relaxed(&loc->inuse_bytes);

      const size_t frame_count = (loc->callstack.count > MI_MAX_BACKTRACE_DEPTH ? MI_MAX_BACKTRACE_DEPTH : loc->callstack.count);
      uint64_t location_ids[MI_MAX_BACKTRACE_DEPTH];
      for (size_t j = 0; j < frame_count; j++) {
        const mi_addr_t addr = mi_addr(loc->callstack.frames[j]);
        bool is_new = false;
        const uint64_t id = mi_pb_addr_map_find_or_insert(addr_map, addr, next_location_id, &is_new);
        if (addr_map->failed) return false;
        location_ids[j] = id;
        if (is_new) {
          next_location_id++;
          mi_pb_buf_t location;
          mi_pb_init(&location, heap);
          mi_pb_uint64_field(&location, 1, id);                                    // id
          mi_pb_uint64_field(&location, 2, mi_pprof_find_mapping_id(mods, addr));   // mapping_id
          mi_pb_uint64_field(&location, 3, (uint64_t)addr);                        // address
          mi_pb_message_field(profile, 4, &location);
          mi_pb_done(&location);
        }
      }

      mi_pb_buf_t sample;
      mi_pb_init(&sample, heap);
      mi_pb_packed_uint64_field(&sample, 1, location_ids, frame_count);  // location_id (leaf-first, matching our capture order)
      int64_t values[4] = {
        loc->alloc_count, loc->alloc_bytes,
        (int64_t)inuse_count, (int64_t)inuse_bytes
      };
      mi_pprof_unsample(&values[0], &values[1], prof->sample_threshold);
      mi_pprof_unsample(&values[2], &values[3], prof->sample_threshold);
      mi_pb_packed_int64_field(&sample, 2, values, 4);  // value
      {
        // attach the originating thread id as a numeric label: pprof has no
        // dedicated thread-id field, but `Sample.label` is the generic
        // mechanism for this kind of per-sample metadata.
        mi_pb_buf_t label;
        mi_pb_init(&label, heap);
        mi_pb_int64_field(&label, 1, str_thread);            // key
        mi_pb_int64_field(&label, 3, (int64_t)loc->thread_id); // num
        mi_pb_message_field(&sample, 3, &label);
        mi_pb_done(&label);
      }
      mi_pb_message_field(profile, 2, &sample);
      mi_pb_done(&sample);
      if (profile->failed) return false;
    }
  }

  // Profile.period_type = 11 / Profile.period = 12: matches the convention
  // used by Go's heap profiles ("space"/"bytes", sampling period in bytes).
  {
    mi_pb_buf_t period_type;
    mi_pb_init(&period_type, heap);
    mi_pb_int64_field(&period_type, 1, str_space);
    mi_pb_int64_field(&period_type, 2, str_bytes);
    mi_pb_message_field(profile, 11, &period_type);
    mi_pb_done(&period_type);
  }
  mi_pb_int64_field(profile, 12, (int64_t)prof->sample_threshold);      // period
  const int64_t elapsed_nanos = (int64_t)(_mi_clock_now() - prof->start_clock) * 1000000LL;  // clock is in milli-secs
  mi_pb_int64_field(profile, 9, prof->start_time_secs * 1000000000LL + elapsed_nanos);  // time_nanos
  mi_pb_int64_field(profile, 10, elapsed_nanos);                                       // duration_nanos

  // the string table (Profile.string_table = 6) can only be finalized now,
  // since strings were interned into it while building the message above.
  if (strings->failed) return false;
  mi_pb_bytes(profile, strings->table.data, strings->table.size);
  if (profile->failed) return false;
  return true;
}

static bool mi_pprof_write_file(mi_heap_t* heap, const char* fname, const mi_pb_buf_t* profile) {
  mi_pprof_file_t* f = _mi_pprof_fopen(heap, fname, true);
  if (f == NULL) return false;
  _mi_pprof_fwrite(f, profile->data, profile->size);
  return _mi_pprof_fclose(f);
}

// Write out the current profiler data as a protobuf `Profile` message.
// Returns `false` if any allocation failed (in which case no file is written):
// the snapshot is best effort and must never crash the profiled program.
static bool mi_pprof_snapshot_proto(mi_pprof_profiler_t* prof, const char* fname) {
  mi_heap_t* heap = prof->profile_heap;
  mi_pb_buf_t profile;
  mi_pb_init(&profile, heap);
  mi_pb_strings_t strings;
  mi_pb_strings_init(&strings, heap);
  mi_pprof_modules_t mods;
  mods.modules = NULL;
  mods.count = 0;
  mods.capacity = 0;
  mods.failed = false;
  mi_pb_addr_map_t addr_map;
  addr_map.heap = heap;
  addr_map.bucket_count = 0;
  addr_map.buckets = NULL;
  addr_map.failed = false;

  bool ok = mi_pprof_build_proto(prof, &profile, &strings, &mods, &addr_map);
  mi_pb_addr_map_done(&addr_map);
  _mi_pprof_modules_done(&mods);
  mi_pb_strings_done(&strings);
  if (ok) { ok = mi_pprof_write_file(heap, fname, &profile); }
  mi_pb_done(&profile);
  return ok;
}

#endif  // MI_PROFILE_USE_BUILTIN
