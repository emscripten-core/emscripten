/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "mimalloc.h"
#include "mimalloc/prim/prim.h"  // mi_prim_out_stderr
#include "mimalloc/prim/prim-mte.h"  // mi_memtag_is_enabled, MI_MEMTAG_MODE_*
#include "mimalloc/prim/prim-tls.h"
#include "mimalloc/alloc/page.h"
#include "mimalloc/util/options.h"
#include "mimalloc/heap/process.h"

#include <stdio.h>      // stdin/stdout
#include <stdlib.h>     // abort

static void mi_add_stderr_output(void);

int mi_version(void) {
  return MI_MALLOC_VERSION;
}


// --------------------------------------------------------
// Options
// These can be accessed by multiple threads and may be
// concurrently initialized or set. The value is relaxed atomic,
// and the init state is release/acquire so a reader that sees
// an initialized state also sees the initialized value.
// --------------------------------------------------------


#define MI_OPTION(opt)                  mi_option_##opt, #opt, NULL
#define MI_OPTION_LEGACY(opt,legacy)    mi_option_##opt, #opt, #legacy

// Some options can be set at build time for statically linked libraries
// (use `-DMI_EXTRA_CPPDEFS="opt1=val1;opt2=val2"`)
//
// This is useful if we cannot pass them as environment variables
// (and setting them programmatically would be too late)

#ifndef MI_DEFAULT_VERBOSE
#define MI_DEFAULT_VERBOSE 0
#endif

#ifndef MI_DEFAULT_ARENA_EAGER_COMMIT
#define MI_DEFAULT_ARENA_EAGER_COMMIT 2
#endif

// in KiB
#ifndef MI_DEFAULT_ARENA_RESERVE
 #if (MI_SIZE_SIZE>4)
  #define MI_DEFAULT_ARENA_RESERVE 1024L*1024L
 #else
  #define MI_DEFAULT_ARENA_RESERVE 128L*1024L
 #endif
#endif

#ifndef MI_DEFAULT_ARENA_MAX_OBJECT_SIZE
#define MI_DEFAULT_ARENA_MAX_OBJECT_SIZE   ((MI_SIZE_BITS * MI_ARENA_MAX_CHUNK_OBJ_SIZE)/MI_KiB)  /* 2 GiB (or 256 MiB on 32-bit), larger than this is alloc'd by the OS */
#endif

#ifndef MI_DEFAULT_DISALLOW_ARENA_ALLOC
#define MI_DEFAULT_DISALLOW_ARENA_ALLOC 0
#endif

#ifndef MI_DEFAULT_ALLOW_LARGE_OS_PAGES
#define MI_DEFAULT_ALLOW_LARGE_OS_PAGES 0
#endif

#ifndef MI_DEFAULT_RESERVE_HUGE_OS_PAGES
#define MI_DEFAULT_RESERVE_HUGE_OS_PAGES 0
#endif

#ifndef MI_DEFAULT_RESERVE_OS_MEMORY
#define MI_DEFAULT_RESERVE_OS_MEMORY 0
#endif

#ifndef MI_DEFAULT_GUARDED_SAMPLE_RATE
#if MI_GUARDED && MI_DEBUG
#define MI_DEFAULT_GUARDED_SAMPLE_RATE  0  /* MI_MiB */
#else
#define MI_DEFAULT_GUARDED_SAMPLE_RATE  0
#endif
#endif

#ifndef MI_DEFAULT_PAGEMAP_COMMIT
#define MI_DEFAULT_PAGEMAP_COMMIT 0
#endif

#ifndef MI_DEFAULT_PAGE_MAX_RECLAIM
#define MI_DEFAULT_PAGE_MAX_RECLAIM  (-1)               // unlimited
#endif

#ifndef MI_DEFAULT_PAGE_FULL_RETAIN
#define MI_DEFAULT_PAGE_FULL_RETAIN  2
#endif

#ifndef MI_DEFAULT_PAGE_CROSS_THREAD_MAX_RECLAIM
#define MI_DEFAULT_PAGE_CROSS_THREAD_MAX_RECLAIM  32
#endif

#ifndef MI_DEFAULT_ALLOW_THP
#if defined(__ANDROID__)
#define MI_DEFAULT_ALLOW_THP  0
#else
// #define MI_DEFAULT_ALLOW_THP  1    // allow THP but purging may split up THP pages
#define MI_DEFAULT_ALLOW_THP  2       // allow THP and set the minimal purge size to 2MiB to avoid breaking them up
#endif
#endif

#ifndef MI_DEFAULT_STATS_MERGE_ON_COLLECT
#define MI_DEFAULT_STATS_MERGE_ON_COLLECT  1
#endif

#ifndef MI_DEFAULT_STATS_MERGE_THRESHOLD
#define MI_DEFAULT_STATS_MERGE_THRESHOLD   (4*1024*1024)  // 4 MiB
#endif

#ifndef MI_DEFAULT_ARENA_PURGE_IMMEDIATE_SIZE
#define MI_DEFAULT_ARENA_PURGE_IMMEDIATE_SIZE  ((MI_SIZE_SIZE * MI_LARGE_PAGE_SIZE) / MI_KiB)  // 32 MiB (in KiB)
#endif

// Static option descriptions (the current values are in `_mi_process.options`)
static const mi_option_desc_t mi_option_descs[_mi_option_last] =
{
  // stable options
#if MI_DEBUG || MI_SHOW_ERRORS
  { 1, MI_OPTION(show_errors) },
#else
  { 0, MI_OPTION(show_errors) },
#endif
  { 0, MI_OPTION(show_stats) },
  { MI_DEFAULT_VERBOSE, MI_OPTION(verbose) },

  // some of the following options are experimental and not all combinations are allowed.
  { 1, MI_OPTION(deprecated_eager_commit) },
  { MI_DEFAULT_ARENA_EAGER_COMMIT, MI_OPTION_LEGACY(arena_eager_commit,eager_region_commit) }, // eager commit arena's? 2 is used to enable this only on an OS that has overcommit (i.e. linux)
  { 1, MI_OPTION_LEGACY(purge_decommits,reset_decommits) },        // purge decommits memory (instead of reset) (note: on linux this uses MADV_DONTNEED for decommit)
  { MI_DEFAULT_ALLOW_LARGE_OS_PAGES, MI_OPTION_LEGACY(allow_large_os_pages,large_os_pages) },    // use large OS pages, use only with eager commit to prevent fragmentation of VMA's
  { MI_DEFAULT_RESERVE_HUGE_OS_PAGES, MI_OPTION(reserve_huge_os_pages) },      // per 1GiB huge pages
  {-1, MI_OPTION(reserve_huge_os_pages_at) },   // reserve huge pages at node N
  { MI_DEFAULT_RESERVE_OS_MEMORY, MI_OPTION(reserve_os_memory)     },      // reserve N KiB OS memory in advance (use `option_get_size`)
  { 0, MI_OPTION(deprecated_segment_cache) },   // cache N segments per thread
  { 0, MI_OPTION(deprecated_page_reset) },      // reset page memory on free
  { 0, MI_OPTION(deprecated_abandoned_page_purge) },
  { 0, MI_OPTION(deprecated_segment_reset) },   // reset segment memory on free (needs eager commit)
  { 1, MI_OPTION(deprecated_eager_commit_delay) },
  { 1000, MI_OPTION_LEGACY(purge_delay,reset_delay) },  // purge delay in milli-seconds
  { 0, MI_OPTION(use_numa_nodes) },           // 0 = use available numa nodes, otherwise use at most N nodes.
  { 0, MI_OPTION_LEGACY(disallow_os_alloc,limit_os_alloc) },           // 1 = do not use OS memory for allocation (but only reserved arenas)
  { 100, MI_OPTION(os_tag) },                   // only apple specific for now but might serve more or less related purpose
  { 32, MI_OPTION(max_errors) },               // maximum errors that are output
  { 32, MI_OPTION(max_warnings) },             // maximum warnings that are output
  { 10, MI_OPTION(deprecated_max_segment_reclaim)},       // max. percentage of the abandoned segments to be reclaimed per try.
  { 0, MI_OPTION(destroy_on_exit)},           // release all OS memory on process exit; careful with dangling pointer or after-exit frees!
  { MI_DEFAULT_ARENA_RESERVE, MI_OPTION(arena_reserve) }, // reserve memory N KiB at a time (=1GiB) (use `option_get_size`)
  { 4, MI_OPTION(arena_purge_mult) },         // purge delay multiplier for arena's
  { 1, MI_OPTION_LEGACY(deprecated_purge_extend_delay, decommit_extend_delay) },
  { MI_DEFAULT_DISALLOW_ARENA_ALLOC, MI_OPTION(disallow_arena_alloc) }, // 1 = do not use arena's for allocation (except if using specific arena id's)
  { 400, MI_OPTION(retry_on_oom) },             // windows only: retry on out-of-memory for N milli seconds (=400), set to 0 to disable retries.
  { 1, MI_OPTION(deprecated_visit_abandoned) },
  { 0, MI_OPTION(guarded_min) },              // only used when building with MI_GUARDED: minimal rounded object size for guarded objects
  { MI_GiB, MI_OPTION(guarded_max) },           // only used when building with MI_GUARDED: maximal rounded object size for guarded objects
  { 0, MI_OPTION(deprecated_unsafe_guarded_precise) },          // deprecated, see MI_UNSAFE_GUARDED_PRECISE macro
  { MI_DEFAULT_GUARDED_SAMPLE_RATE, MI_OPTION(guarded_sample_rate)},       // guard an object in the min/max range about every N requested bytes (=0)
  { 0, MI_OPTION(guarded_sample_seed)},
  { 10000, MI_OPTION(generic_collect) },        // collect theaps every N (=10000) generic allocation calls
  { 0, MI_OPTION_LEGACY(page_reclaim_on_free, abandoned_reclaim_on_free) },// reclaim abandoned (small) pages on a free: -1 = disable completely, 0 (default) = only reclaim into the originating theap, 1 = reclaim on free across theaps
  { MI_DEFAULT_PAGE_FULL_RETAIN, MI_OPTION(page_full_retain) },         // number of (small) pages to retain in the free page queues
  { 4, MI_OPTION(page_max_candidates) },      // max search to find a best page candidate
  { 0, MI_OPTION(max_vabits) },               // max virtual address space bits
  { MI_DEFAULT_PAGEMAP_COMMIT, MI_OPTION(pagemap_commit) },           // commit the full pagemap upfront?
  { 0, MI_OPTION(page_commit_on_demand) },    // commit pages on-demand (2 disables this only on overcommit systems (like Linux))
  { MI_DEFAULT_PAGE_MAX_RECLAIM, MI_OPTION(page_max_reclaim) },         // don't reclaim (small) pages of the same originating theap if we already own N pages in that size class
  { MI_DEFAULT_PAGE_CROSS_THREAD_MAX_RECLAIM, MI_OPTION(page_cross_thread_max_reclaim) }, // don't reclaim (small) pages across threads if we already own N pages in that size class
  { MI_DEFAULT_ALLOW_THP, MI_OPTION(allow_thp) },                // transparent huge pages: 0 disable for the process, 1 allow, 2 allow and avoid splitting on purge.
  { 0, MI_OPTION_LEGACY(arena_purge_min_size, minimal_purge_size) },       // set minimal purge size (in KiB) (=0). Using 0 resolves to either 64 (or 2048 if THP is enabled).
  { MI_DEFAULT_ARENA_MAX_OBJECT_SIZE, MI_OPTION(arena_max_object_size) },    // set maximal object size that can be allocated in an arena (in KiB) (=2GiB on 64-bit).
  { 0, MI_OPTION(arena_is_numa_local) },      // associate local numa node with an initial arena allocation
  { MI_DEFAULT_STATS_MERGE_ON_COLLECT, MI_OPTION(stats_merge_on_collect) },   // on each theap collect, stats are merged with the parent heap
  { MI_GiB, MI_OPTION(profile_alloc_interval) },// N bytes (=1 GiB default) between automatic dumps of the `MIMALLOC_PROFILE` heap profiler (use `option_get_size`); 0 disables automatic dumping
  { 0, MI_OPTION(profile_inuse_interval) },   // N bytes between automatic dumps of the `MIMALLOC_PROFILE` heap profiler whenever in-use bytes grow by that amount (use `option_get_size`); 0 disables (default)
  { 0, MI_OPTION(profile_time_interval) },    // N seconds between automatic dumps of the `MIMALLOC_PROFILE` heap profiler; 0 disables (default)
  { 512*MI_KiB, MI_OPTION(profile_sample_rate) }, // sample rate in bytes for the `MIMALLOC_PROFILE` heap profiler (use `option_get_size`) (=512 KiB)
  { MI_DEFAULT_STATS_MERGE_THRESHOLD, MI_OPTION(stats_merge_threshold) },    // on a page stats update, merge theap stats into the heap if N bytes were freed/allocated; 0 disables (=4 MiB)
  { MI_DEFAULT_ARENA_PURGE_IMMEDIATE_SIZE, MI_OPTION(arena_purge_immediate_size) }, // ranges above this size are purged immediately
  { 1, MI_OPTION(mte) }                       // use memory tagging (MTE) if available (and built with `MI_MTE=PREFER` or `ON`); on Linux the tag check mode can be set as well (sync, async, preferred)
};

static void mi_option_init(mi_option_t option, mi_option_state_t* state);
static bool mi_options_init_from_environ(void);

static const mi_option_desc_t* mi_option_desc(mi_option_t option) {
  mi_assert_internal((int)option >= 0 && (int)option < (int)_mi_option_last);
  const mi_option_desc_t* desc = &mi_option_descs[option];
  mi_assert(desc->option == option);  // index should match the option
  return desc;
}

static inline long mi_option_state_value(mi_option_state_t* state) {
  return (long)mi_atomic_loads_relaxed(&state->value);
}

static inline mi_option_init_t mi_option_state_init(mi_option_state_t* state) {
  return (mi_option_init_t)mi_atomic_load_acquire(&state->init);
}

// Set the value and then the init state (so readers that see the init state also see the value).
static inline void mi_option_state_set(mi_option_state_t* state, long value, mi_option_init_t init) {
  mi_atomic_storess_relaxed(&state->value, (mi_ssize_t)value);
  mi_atomic_store_release(&state->init, (size_t)init);
}

// Get the option state with the value set (to the default if not yet set) (but not reading from the environment)
static mi_option_state_t* mi_option_state(mi_option_t option) {
  mi_option_state_t* state = &_mi_process.options[option];
  if mi_unlikely(mi_option_state_init(state) == MI_OPTION_UNINIT) {
    mi_option_state_set(state, mi_option_desc(option)->default_value, MI_OPTION_PENDING);
  }
  return state;
}

// option value is stored in KiB so it can fit a `long`
// todo: use mi_ssize_t for options?
static bool mi_option_has_size_in_kib(mi_option_t option) {
  return (option == mi_option_reserve_os_memory || option == mi_option_arena_reserve ||
          option == mi_option_arena_purge_min_size || option == mi_option_arena_purge_immediate_size ||
          option == mi_option_arena_max_object_size
          // || option == mi_option_profile_alloc_interval || option == mi_option_profile_inuse_interval || option == mi_option_profile_sample_rate
         );
}

// option value is stored in bytes as a `long` is enough.
static bool mi_option_has_size_in_bytes(mi_option_t option) {
  return (option == mi_option_stats_merge_threshold ||
          option == mi_option_profile_alloc_interval || option == mi_option_profile_inuse_interval || option == mi_option_profile_sample_rate);
}

void _mi_options_init(void) {
  // called on process load
  if (!mi_options_init_from_environ()) {
    // the full environment is not available: look up each option by name
    for(int i = 0; i < _mi_option_last; i++ ) {
      mi_option_t option = (mi_option_t)i;
      long l = mi_option_get(option); MI_UNUSED(l); // initialize
    }
  }
  #if MI_GUARDED
  if (mi_option_get(mi_option_guarded_sample_rate) > 0) {
    if (mi_option_is_enabled(mi_option_allow_large_os_pages)) {
      mi_option_disable(mi_option_allow_large_os_pages);
      _mi_warning_message("option 'allow_large_os_pages' is disabled to allow for guarded objects\n");
    }
  }
  #endif
}

// called at actual process load, it should be safe to print now
void _mi_options_post_init(void) {
  mi_add_stderr_output(); // now it safe to use stderr for output
  if (mi_option_is_enabled(mi_option_verbose)) { mi_options_print(); }
}

#define mi_stringifyx(str)  #str                // and stringify
#define mi_stringify(str)   mi_stringifyx(str)  // expand

mi_decl_export void mi_options_print_out(mi_output_fun* out, void* arg) mi_attr_noexcept
{
  // show version
  const int vermajor = MI_MALLOC_VERSION/10000;
  const int verminor = (MI_MALLOC_VERSION%10000)/100;
  const int verpatch = (MI_MALLOC_VERSION%100);
  _mi_fprintf(out, arg, "v%i.%i.%i%s%s\n", vermajor, verminor, verpatch,
      #if defined(MI_CMAKE_BUILD_TYPE)
      ", " mi_stringify(MI_CMAKE_BUILD_TYPE)
      #else
      ""
      #endif
      ,
      #if defined(MI_GIT_DESCRIBE)
      ", git " mi_stringify(MI_GIT_DESCRIBE)
      #else
      ""
      #endif
      );

  // show options
  for (int i = 0; i < _mi_option_last; i++) {
    mi_option_t option = (mi_option_t)i;
    long l = mi_option_get(option); // possibly initialize
    _mi_fprintf(out, arg, "option '%s': %ld %s\n", mi_option_desc(option)->name, l, (mi_option_has_size_in_kib(option) ? "KiB" : (mi_option_has_size_in_bytes(option) ? "B" : "")));
  }

  // show build configuration
  #if MI_PAGE_META_IS_ALIGNED && MI_PAGE_META_SMALL_IS_ALIGNED
  _mi_fprintf(out, arg, "free mode   : (small) aligned, page size: %zu\n", sizeof(mi_page_t));
  #elif MI_PAGE_META_IS_ALIGNED
  _mi_fprintf(out, arg, "free mode   : aligned, page size: %zu\n", sizeof(mi_page_t));
  #elif MI_PAGE_META_SMALL_IS_ALIGNED
  _mi_fprintf(out, arg, "free mode   : small aligned + pagemap, page size: %zu\n", sizeof(mi_page_t));
  #elif MI_FREE_IS_CHECKED
  _mi_fprintf(out, arg, "free mode   : checked, page size: %zu\n", sizeof(mi_page_t));
  #else
  _mi_fprintf(out, arg, "free mode   : pagemap, page size: %zu\n", sizeof(mi_page_t));
  #endif
  #if MI_ENCODE_FREELIST
  _mi_fprintf(out, arg, "free lists  : encoded (%s)\n", (MI_SIZE_BITS>=64 ? (MI_ENCODE_FREELIST==1 && MI_PAC<2 ? "tagged offsets, low mul" : "tagged offsets") : "xor-shift"));
  #endif
  #if MI_GUARDED
  _mi_fprintf(out, arg, "guarded mode: %s, rate=%ld\n",  MI_SAMPLE==2 ? "fine-grained" : "enabled", mi_option_get(mi_option_guarded_sample_rate));
  #endif
  #if MI_PROFILE
  _mi_fprintf(out, arg, "profiling   : %s\n", MI_SAMPLE==2 ? "fine-grained" : "enabled");
  #endif
  #if MI_TSAN
  _mi_fprintf(out, arg, "thread santizer enabled\n");
  #endif
  #if MI_MTE
  if (mi_memtag_is_enabled()) { _mi_fprintf(out, arg, "mem tagging : enabled\n"); }
  else { _mi_fprintf(out, arg, "mem tagging : disabled\n"); }
  #endif
  #if MI_PAC
  _mi_fprintf(out, arg, "ptr signing : %s\n", (MI_PAC>=2 ? "enabled (pacga)" : "weak (process key)"));
  #endif
  _mi_fprintf(out, arg, "mem tracking: %s\n", MI_TRACK_TOOL);
  _mi_fprintf(out, arg, "debug level : %d\n", MI_DEBUG );
  _mi_fprintf(out, arg, "secure level: %d\n", MI_SECURE );
  _mi_fprintf(out, arg, "padding     : %s\n", (MI_PADDING==0 ? "none" : (MI_PADDING==1 ? "enabled" : "byte precise")));
}

mi_decl_export void mi_options_print(void) mi_attr_noexcept {
  mi_options_print_out(NULL, NULL);
}

long _mi_option_get_fast(mi_option_t option) {
  mi_assert(option >= 0 && option < _mi_option_last);
  return mi_option_state_value(mi_option_state(option));
}


mi_decl_nodiscard long mi_option_get(mi_option_t option) {
  mi_assert(option >= 0 && option < _mi_option_last);
  if (option < 0 || option >= _mi_option_last) return 0;
  mi_option_state_t* state = &_mi_process.options[option];
  if mi_unlikely(mi_option_state_init(state) < MI_OPTION_DEFAULTED) {  // uninit or pending
    state = mi_option_state(option);
    mi_option_init(option, state);
  }
  return mi_option_state_value(state);
}

mi_decl_nodiscard long mi_option_get_clamp(mi_option_t option, long min, long max) {
  long x = mi_option_get(option);
  return (x < min ? min : (x > max ? max : x));
}

mi_decl_nodiscard size_t mi_option_get_size(mi_option_t option) {
  const long x = mi_option_get(option);
  size_t size = (x < 0 ? 0 : (size_t)x);
  if (mi_option_has_size_in_kib(option)) {
    if (mi_mul_overflow(size, MI_KiB, &size)) {
      size = MI_MAX_ALLOC_SIZE;
    }
  }
  return size;
}

void mi_option_set(mi_option_t option, long value) {
  mi_assert(option >= 0 && option < _mi_option_last);
  if (option < 0 || option >= _mi_option_last) return;
  mi_option_state_t* const state = &_mi_process.options[option];
  mi_option_state_set(state, value, MI_OPTION_INITIALIZED);
  // ensure min/max range; be careful to not recurse.
  if (option == mi_option_guarded_min && mi_option_state_value(mi_option_state(mi_option_guarded_max)) < value) {
    mi_option_set(mi_option_guarded_max, value);
  }
  else if (option == mi_option_guarded_max && mi_option_state_value(mi_option_state(mi_option_guarded_min)) > value) {
    mi_option_set(mi_option_guarded_min, value);
  }
}

void mi_option_set_default(mi_option_t option, long value) {
  mi_assert(option >= 0 && option < _mi_option_last);
  if (option < 0 || option >= _mi_option_last) return;
  mi_option_state_t* const state = mi_option_state(option);
  if (mi_option_state_init(state) != MI_OPTION_INITIALIZED) {
    mi_atomic_storess_relaxed(&state->value, (mi_ssize_t)value);
  }
}

mi_decl_nodiscard bool mi_option_is_enabled(mi_option_t option) {
  return (mi_option_get(option) != 0);
}

void mi_option_set_enabled(mi_option_t option, bool enable) {
  mi_option_set(option, (enable ? 1 : 0));
}

void mi_option_set_enabled_default(mi_option_t option, bool enable) {
  mi_option_set_default(option, (enable ? 1 : 0));
}

void mi_option_enable(mi_option_t option) {
  mi_option_set_enabled(option,true);
}

void mi_option_disable(mi_option_t option) {
  mi_option_set_enabled(option,false);
}

static void mi_cdecl mi_out_stderr(const char* msg, void* arg) {
  MI_UNUSED(arg);
  if (msg != NULL && msg[0] != 0) {
    _mi_prim_out_stderr(msg);
  }
}

// Since an output function can be registered earliest in the `main`
// function we also buffer output that happens earlier. When
// an output function is registered it is called immediately with
// the output up to that point.
// The buffer is `_mi_process.output_buf` but the lock is static as it is used before process initialization
// and a zero lock is invalid on some platforms.
static mi_lock_t out_buf_lock = MI_LOCK_INITIALIZER;

static void mi_cdecl mi_out_buf(const char* msg, void* arg) {
  MI_UNUSED(arg);
  if (msg==NULL) return;
  if (mi_atomic_load_acquire(&_mi_process.output_buf_len)>=MI_MAX_DELAY_OUTPUT) return;
  size_t n = _mi_strlen(msg);
  if (n==0 || n >= MI_MAX_DELAY_OUTPUT) return;
  // copy msg into the buffer
  mi_lock(&out_buf_lock) {
    const size_t start = mi_atomic_add_acq_rel(&_mi_process.output_buf_len, n);
    if (start < MI_MAX_DELAY_OUTPUT) {
      // check bound
      if (start+n >= MI_MAX_DELAY_OUTPUT) {
        n = MI_MAX_DELAY_OUTPUT-start-1;
      }
      mi_memcpy(&_mi_process.output_buf[start], msg, n);
    }
  }
}

static void mi_out_buf_flush(mi_output_fun* out, bool no_more_buf, void* arg) {
  if (out==NULL) return;
  // claim (if `no_more_buf == true`, no more output will be added after this point)
  mi_lock(&out_buf_lock) {
    size_t count = mi_atomic_add_acq_rel(&_mi_process.output_buf_len, (no_more_buf ? MI_MAX_DELAY_OUTPUT : 1));
    // and output the current contents
    if (count>MI_MAX_DELAY_OUTPUT) count = MI_MAX_DELAY_OUTPUT;
    _mi_process.output_buf[count] = 0;
    out(_mi_process.output_buf,arg);
    if (!no_more_buf) {
      _mi_process.output_buf[count] = '\n'; // if continue with the buffer, insert a newline
    }
  }
}


// Once this module is loaded, switch to this routine
// which outputs to stderr and the delayed output buffer.
static void mi_cdecl mi_out_buf_stderr(const char* msg, void* arg) {
  mi_out_stderr(msg,arg);
  mi_out_buf(msg,arg);
}



// --------------------------------------------------------
// Default output handler
// --------------------------------------------------------

// The output function and argument are stored in the sealed process data (see `heap/process.h`).
static mi_output_fun* mi_out_get_default(void** parg) {
  const mi_process_sealed_t* const sealed = mi_process_get_sealed();
  void* arg;
  mi_output_fun* const out = (mi_output_fun*)_mi_process_get_callback(&sealed->out_default, &sealed->out_arg, &arg);
  if (parg != NULL) { *parg = arg; }
  return (out == NULL ? &mi_out_buf : out);
}

void mi_register_output(mi_output_fun* out, void* arg) mi_attr_noexcept {
  const mi_process_sealed_t* const sealed = mi_process_get_sealed();
  _mi_process_set_callback(&sealed->out_default, &sealed->out_arg, (void*)(out == NULL ? &mi_out_stderr : out), arg); // stop using the delayed output buffer
  if (out!=NULL) { mi_out_buf_flush(out,true,arg); }        // output all the delayed output now
}

// add stderr to the delayed output after the module is loaded
static void mi_add_stderr_output(void) {
  mi_assert_internal(mi_process_get_sealed()->out_default == NULL);
  mi_out_buf_flush(&mi_out_stderr, false, NULL); // flush current contents to stderr
  const mi_process_sealed_t* const sealed = mi_process_get_sealed();
  _mi_process_set_callback(&sealed->out_default, &sealed->out_arg, (void*)&mi_out_buf_stderr, NULL);  // and add stderr to the delayed output
}

// --------------------------------------------------------
// Messages, all end up calling `_mi_fputs`.
// --------------------------------------------------------
static mi_decl_noinline bool mi_recurse_enter_prim(void) {
  return true;
}

static mi_decl_noinline void mi_recurse_exit_prim(void) {
  /* nothing */
}

static bool mi_recurse_enter(void) {
  #if defined(__APPLE__) || defined(__ANDROID__) || MI_TLS_RECURSE_GUARD
  if (_mi_preloading()) return false;
  #endif
  return mi_recurse_enter_prim();
}

static void mi_recurse_exit(void) {
  #if defined(__APPLE__) || defined(__ANDROID__) || MI_TLS_RECURSE_GUARD
  if (_mi_preloading()) return;
  #endif
  mi_recurse_exit_prim();
}

void _mi_fputs(mi_output_fun* out, void* arg, const char* prefix, const char* message) {
  if (out==NULL || (void*)out==(void*)stdout || (void*)out==(void*)stderr) { // todo: use mi_out_stderr for stderr?
    if (!mi_recurse_enter()) return;
    out = mi_out_get_default(&arg);
    if (prefix != NULL) out(prefix, arg);
    out(message, arg);
    mi_recurse_exit();
  }
  else {
    if (prefix != NULL) out(prefix, arg);
    out(message, arg);
  }
}

// Define our own limited `fprintf` that avoids memory allocation.
// We do this using `_mi_vsnprintf` with a limited buffer.
static void mi_vfprintf( mi_output_fun* out, void* arg, const char* prefix, const char* fmt, va_list args ) {
  char buf[992];
  if (fmt==NULL) return;
  if (!mi_recurse_enter()) return;
  _mi_vsnprintf(buf, sizeof(buf)-1, fmt, args);
  mi_recurse_exit();
  _mi_fputs(out,arg,prefix,buf);
}

void _mi_fprintf( mi_output_fun* out, void* arg, const char* fmt, ... ) {
  va_list args;
  va_start(args,fmt);
  mi_vfprintf(out,arg,NULL,fmt,args);
  va_end(args);
}

static void mi_vfprintf_thread(mi_output_fun* out, void* arg, const char* prefix, const char* fmt, va_list args) {
  if (prefix != NULL && _mi_strnlen(prefix,33) <= 32) { // && !_mi_is_main_thread()) {
    char tprefix[64];
    _mi_snprintf(tprefix, sizeof(tprefix), "%sthread %p: ", prefix, (void*)_mi_thread_id());
    mi_vfprintf(out, arg, tprefix, fmt, args);
  }
  else {
    mi_vfprintf(out, arg, prefix, fmt, args);
  }
}

void _mi_raw_message(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  mi_vfprintf(NULL, NULL, NULL, fmt, args);
  va_end(args);
}

void _mi_trace_message(const char* fmt, ...) {
  if (mi_option_get(mi_option_verbose) <= 1) return;  // only with verbose level 2 or higher
  va_list args;
  va_start(args, fmt);
  mi_vfprintf_thread(NULL, NULL, "mimalloc: ", fmt, args);
  va_end(args);
}

void _mi_verbose_message(const char* fmt, ...) {
  if (!mi_option_is_enabled(mi_option_verbose)) return;
  va_list args;
  va_start(args,fmt);
  mi_vfprintf(NULL, NULL, "mimalloc: ", fmt, args);
  va_end(args);
}

static void mi_show_error_message(const char* fmt, va_list args) {
  if (!mi_option_is_enabled(mi_option_verbose)) {
    if (!mi_option_is_enabled(mi_option_show_errors)) return;
    const long max_errors = mi_option_get(mi_option_max_errors);
    if (max_errors >= 0 && (long)mi_atomic_increment_acq_rel(&_mi_process.error_count) >= max_errors) return;
  }
  mi_vfprintf_thread(NULL, NULL, "mimalloc: error: ", fmt, args);
}

void _mi_warning_message(const char* fmt, ...) {
  if (!mi_option_is_enabled(mi_option_verbose)) {
    if (!mi_option_is_enabled(mi_option_show_errors)) return;
    const long max_warnings = mi_option_get(mi_option_max_warnings);
    if (max_warnings >= 0 && (long)mi_atomic_increment_acq_rel(&_mi_process.warning_count) >= max_warnings) return;
  }
  va_list args;
  va_start(args,fmt);
  mi_vfprintf_thread(NULL, NULL, "mimalloc: warning: ", fmt, args);
  va_end(args);
}


// flush delayed output (if not yet initialized) so a serious error message is not lost
static void mi_out_buf_flush_now(void) {
  if (mi_atomic_load_ptr_acquire(void,&mi_process_get_sealed()->out_default) == NULL) { mi_out_buf_flush(&mi_out_stderr, true, NULL); }
}

#if MI_DEBUG
mi_decl_noreturn mi_decl_cold void _mi_assert_fail(const char* assertion, const char* fname, unsigned line, const char* func ) mi_attr_noexcept {
  _mi_fprintf(NULL, NULL, "mimalloc: assertion failed: at \"%s\":%u, %s\n  assertion: \"%s\"\n", fname, line, (func==NULL?"":func), assertion);
  mi_out_buf_flush_now();
  abort();
}
#endif

// --------------------------------------------------------
// Errors
// --------------------------------------------------------

// The error handler and argument are stored in the sealed process data (see `heap/process.h`).
static void mi_error_default(int err) {
  MI_UNUSED(err);
  #if (MI_DEBUG>0)
    if (err==EFAULT) {
      #ifdef _MSC_VER
      __debugbreak();
      #endif
      abort();
    }
  #endif
  #if MI_XMALLOC
    if (err==ENOMEM || err==EOVERFLOW || err==EINVAL) { // abort on memory allocation fails in xmalloc mode
      abort();
    }
  #endif
  // note: we do not set `errno` here; allocation functions set it themselves on failure (and `free` should not modify it)
}

void mi_register_error(mi_error_fun* fun, void* arg) {
  const mi_process_sealed_t* const sealed = mi_process_get_sealed();
  _mi_process_set_callback(&sealed->error_handler, &sealed->error_arg, (void*)fun, arg);  // can be NULL
}

static void mi_error_message_va(int err, const char* prefix, const char* fmt, va_list args) {
  // show detailed error message; serious errors (EFAULT) are always shown (also before the C runtime is initialized)
  if (err==EFAULT) {
    mi_vfprintf_thread(NULL, NULL, prefix, fmt, args);
    mi_out_buf_flush_now();
  }
  else {
    mi_show_error_message(fmt, args);
  }
  // and call the error handler which may abort (or return normally, potentially setting errno)
  const mi_process_sealed_t* const sealed = mi_process_get_sealed();
  void* arg;
  mi_error_fun* const handler = (mi_error_fun*)_mi_process_get_callback(&sealed->error_handler, &sealed->error_arg, &arg);
  if (handler != NULL) {
    handler(err, arg);
  }
  else {
    mi_error_default(err);
  }
  #if MI_SECURE
  if (err==EFAULT) {  // in secure mode always abort on serious errors (corrupted meta-data), even if the handler returns
    abort();
  }
  #endif
}

void _mi_error_message(int err, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  mi_error_message_va(err, "mimalloc: error: ", fmt, args);
  va_end(args);
}

// Unrecoverable error: report as `EFAULT` and always abort.
mi_decl_noreturn mi_decl_cold void _mi_fatal_error(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  mi_error_message_va(EFAULT, "mimalloc: fatal error: ", fmt, args);
  va_end(args);
  abort();
}

mi_decl_noinline mi_block_t* _mi_error_block_next_is_corrupted(const mi_page_t* page, const mi_block_t* block, mi_encoded_t x) {
  _mi_error_message(EFAULT, "corrupted free list entry at %p in page %p with block size %zu: value %p\n", block, page, mi_page_block_size(page), (void*)x);
  return NULL;
}

mi_decl_noinline void _mi_error_block_next_is_invalid(const mi_page_t* page, const mi_block_t* block, const mi_block_t* next) {
  _mi_error_message(EFAULT, "corrupted free list entry at %p in page %p with block size %zu: invalid link %p\n", block, page, mi_page_block_size(page), next);
}

mi_decl_noinline void* _mi_error_ptr_auth_failed(mi_ptr_signed_t x, mi_addr_t salt) {
  if (x != 0) {
    _mi_error_message(EFAULT, "pointer authentication failed at %p: value %p\n", (void*)salt, (void*)x);
  }
  return NULL;
}

mi_decl_noinline mi_decl_noreturn void _mi_error_list_corrupted(const void* node) {
  _mi_fatal_error("corrupted list links detected at %p\n", node);
}

mi_decl_noinline void _mi_error_ptr_pac_unaligned(const void* p) {
  _mi_fatal_error("internal error: signed pointers must be aligned with memory tagging (%p)\n", p);
}

// --------------------------------------------------------
// Initialize options from the environment
//
// If the platform can iterate the full environment (`_mi_env_visit`), all
// options are initialized in a single pass over the environment (which also
// detects unknown `MIMALLOC_` variables). Otherwise each option is looked up by name
// (`_mi_getenv`) when it is first accessed. Both strategies produce a `mi_option_env_t`
// that is applied to the option state using `mi_option_env_apply` and `mi_option_env_warn`.
// --------------------------------------------------------

// The result of reading an option from the environment
typedef struct mi_option_env_s {
  long  value;    // the parsed value (if `err==0`)
  int   err;      // 0 (found), ENOENT (not found), EINVAL (invalid value), E2BIG (value too long), or EAGAIN (try again later)
  bool  legacy;   // found under its legacy name?
  const char* ignore_reason;
} mi_option_env_t;

#define MI_OPTION_ENV_NOT_FOUND  { 0, ENOENT, false, NULL }

// Parse an environment value `s` for an option; returns 0 on success, EINVAL if invalid, or E2BIG if too long.
static int mi_option_parse(mi_option_t option, const char* s, long* result) {
  char buf[64+1];
  const size_t len = _mi_strnlen(s, sizeof(buf));
  if (len >= sizeof(buf)) return E2BIG;
  for (size_t i = 0; i < len; i++) {
    buf[i] = _mi_toupper(s[i]);
  }
  buf[len] = 0;
  if (buf[0] == 0 || _mi_streq(buf,"1") || _mi_streq(buf,"TRUE") || _mi_streq(buf,"YES") || _mi_streq(buf,"ON")) {
    *result = 1;
    return 0;
  }
  if (_mi_streq(buf,"0") || _mi_streq(buf,"FALSE") || _mi_streq(buf,"NO") || _mi_streq(buf,"OFF")) {
    *result = 0;
    return 0;
  }
  if (option == mi_option_mte && (_mi_streq(buf,"SYNC") || _mi_streq(buf,"ASYNC") || _mi_streq(buf,"PREFERRED"))) {
    *result = (buf[0]=='S' ? MI_MEMTAG_MODE_SYNC : (buf[0]=='A' ? MI_MEMTAG_MODE_ASYNC : MI_MEMTAG_MODE_PREFERRED));
    return 0;
  }
  const char* end = buf;
  long value = 0;
  if (!_mi_strntol(buf, sizeof(buf), &end, &value)) return EINVAL;
  if (mi_option_has_size_in_kib(option) || mi_option_has_size_in_bytes(option)) {
    // the size is interpreted in bytes.
    size_t size = (value < 0 ? 0 : (size_t)value);
    bool overflow = false;
    if (*end == 'K') { overflow = mi_mul_overflow(size,MI_KiB,&size); end++; }
    #if MI_SIZE_SIZE >= 4
    else if (*end == 'M') { overflow = mi_mul_overflow(size,MI_MiB,&size); end++; }
    else if (*end == 'G') { overflow = mi_mul_overflow(size,MI_GiB,&size); end++; }
    #endif
    #if MI_SIZE_SIZE >= 8
    else if (*end == 'T') { overflow = mi_mul_overflow(size,MI_GiB * MI_KiB,&size); end++; }
    #endif
    if (end[0] == 'I' && end[1] == 'B') { end += 2; } // KiB, MiB, GiB, TiB
    else if (*end == 'B') { end++; }                  // Kb, Mb, Gb, Tb
    if (overflow || size > MI_MAX_ALLOC_SIZE) { size = MI_MAX_ALLOC_SIZE; }
    if (mi_option_has_size_in_kib(option)) {
      // This option is interpreted in KiB to prevent overflow of `long` for large allocations
      // (long is 32-bit on 64-bit windows, which allows for 4TiB max.)
      size = (size + MI_KiB - 1) / MI_KiB;  // no overflow due to check before this
    }
    value = (size > LONG_MAX ? LONG_MAX : (long)size);
  }
  if (*end != 0) return EINVAL;
  *result = value;
  return 0;
}

// Apply an environment result to a pending option state (without emitting warnings).
static void mi_option_env_apply(mi_option_t option, mi_option_state_t* state, mi_option_env_t* env) {
  bool ignore_env = false;
  #if MI_SECURE_ENV
  switch (option) {
    case mi_option_show_stats:
    case mi_option_verbose:
    case mi_option_show_errors:
    case mi_option_allow_thp:
      break;
    default:
      ignore_env = true;
      break;
  }
  #elif MI_SECURE >= 1
  switch (option) {
    #if MI_OS_HAS_GUARD_PAGES
    case mi_option_allow_large_os_pages:
    case mi_option_reserve_huge_os_pages:
    #endif
    #if MI_MTE
    case mi_option_mte:
    #endif
    case mi_option_deprecated_unsafe_guarded_precise:
    case mi_option_destroy_on_exit:
      ignore_env = true;
      break;
    default:
      break;
  }
  #endif
  if (env->err != ENOENT && env->err != EAGAIN && (ignore_env || _mi_os_is_elevated())) {
    // don't let environment overrides weaken secure builds or influence a privileged process
    env->ignore_reason = (ignore_env ? (MI_SECURE_ENV ? "secure environment mode" : "secure mode") : "an elevated process");
    env->err = ENOENT;
    env->legacy = false;
  }
  if (env->err == 0) {
    mi_option_set(option, env->value);
  }
  else if (env->err != EAGAIN) {
    mi_atomic_store_release(&state->init, (size_t)MI_OPTION_DEFAULTED);  // not found or invalid: use the default value
  }
  // on EAGAIN, keep pending to try again (can happen during preloading if getenv is not available)
}

// Emit warnings for an applied environment result (after `mi_option_env_apply` so we cannot recurse on a pending option).
static void mi_option_env_warn(mi_option_t option, const mi_option_env_t* env) {
  const mi_option_desc_t* const desc = mi_option_desc(option);
  if (env->ignore_reason != NULL) {
    _mi_warning_message("MIMALLOC_%s is ignored in %s\n", desc->name, env->ignore_reason);
  }
  if (env->legacy && env->err != ENOENT) {
    _mi_warning_message("environment option \"mimalloc_%s\" is deprecated -- use \"mimalloc_%s\" instead.\n", desc->legacy_name, desc->name);
  }
  if (env->err == EINVAL || env->err == E2BIG) {
    // if the 'mimalloc_verbose' env var has a bogus value we'd never know
    // (since the value defaults to 'off') so in that case briefly enable verbose
    mi_option_state_t* const state = &_mi_process.options[option];
    const bool force_verbose = (option == mi_option_verbose && mi_option_state_value(state) == 0);
    if (force_verbose) { mi_atomic_storess_relaxed(&state->value, 1); }
    _mi_warning_message("environment option mimalloc_%s %s.\n", desc->name, (env->err == E2BIG ? "has a value that is too long" : "has an invalid value"));
    if (force_verbose) { mi_atomic_storess_relaxed(&state->value, 0); }
  }
}


// --------------------------------------------------------
// Strategy 1: look up a single option by name
// --------------------------------------------------------

static int mi_option_getenv(const char* name, char* result, size_t result_size) {
  char buf[64+1];
  _mi_strlcpy(buf, "mimalloc_", sizeof(buf));
  _mi_strlcat(buf, name, sizeof(buf));
  return _mi_getenv(buf, result, result_size);
}

static void mi_option_init(mi_option_t option, mi_option_state_t* state) {
  const mi_option_desc_t* const desc = mi_option_desc(option);
  mi_option_env_t env = MI_OPTION_ENV_NOT_FOUND;
  char s[64+1];
  env.err = mi_option_getenv(desc->name, s, sizeof(s));
  if (env.err == ENOENT && desc->legacy_name != NULL) {
    env.err = mi_option_getenv(desc->legacy_name, s, sizeof(s));
    env.legacy = (env.err != ENOENT && env.err != EAGAIN);
  }
  if (env.err == 0) {
    env.err = mi_option_parse(option, s, &env.value);
  }
  mi_option_env_apply(option, state, &env);
  mi_option_env_warn(option, &env);
}


// --------------------------------------------------------
// Strategy 2: initialize all options in one pass over the full environment
// --------------------------------------------------------

typedef struct mi_env_scan_s {
  mi_option_env_t options[_mi_option_last];
  size_t          unknown_count;
  char            unknown[256];   // `;` separated names of unknown `MIMALLOC_` variables (only whole names that fit)
} mi_env_scan_t;

// Is `name` (of length `len`, not zero terminated) equal to `s` (case insensitive)?
static bool mi_env_name_eq(const char* name, size_t len, const char* s) {
  return (s != NULL && _mi_strnicmp(name, s, len) == 0 && s[len] == 0);
}

// Is `name` (without the `mimalloc_` prefix) a known environment variable that is not an option?
static bool mi_env_is_extra(const char* name, size_t len) {
  static const char* const extra_names[] = {
    "profile",    // pprof profiler output (src/profile/pprof.c)
    "defines",    // C defines for the Rust build script (contrib/rust/sys/build.rs)
    "src_dir",    // mimalloc source directory for the Rust build script (contrib/rust/sys/build.rs)
    "disable_redirect", "disable_override", "enable_redirect", "prioritize_load_order",  // used by mimalloc-redirect.dll
    "patch_imports", "force_redirect", "patch_ntheap",
  };
  for (size_t i = 0; i < sizeof(extra_names)/sizeof(extra_names[0]); i++) {
    if (mi_env_name_eq(name, len, extra_names[i])) return true;
  }
  return false;
}

// Append an unknown variable name (of length `len`) to the list of unknown names
static void mi_env_scan_unknown(mi_env_scan_t* scan, const char* name, size_t len) {
  scan->unknown_count++;
  const size_t n = _mi_strnlen(scan->unknown, sizeof(scan->unknown));
  const size_t sep = (n > 0 ? 1 : 0);
  if (n + sep + len < sizeof(scan->unknown)) {
    if (sep > 0) { scan->unknown[n] = ';'; }
    mi_memcpy(scan->unknown + n + sep, name, len);
    scan->unknown[n + sep + len] = 0;
  }
}

// Visit an environment `entry` of the form `name=value`
static void mi_env_scan_entry(const char* entry, void* arg) {
  mi_env_scan_t* const scan = (mi_env_scan_t*)arg;
  const size_t plen = 9;  // `mimalloc_`
  if (_mi_strnicmp(entry, "mimalloc_", plen) != 0) return;
  const char* const name = entry + plen;
  size_t len = 0;
  while (name[len] != 0 && name[len] != '=') { len++; }
  if (name[len] != '=') return;
  const char* const value = name + len + 1;
  for (size_t i = 0; i < _mi_option_last; i++) {
    const mi_option_desc_t* const desc = &mi_option_descs[i];
    mi_option_env_t* const env = &scan->options[i];
    if (mi_env_name_eq(name, len, desc->name)) {
      // the first occurrence takes precedence (as with a lookup by name), but the current name overrides a legacy name
      if (env->err == ENOENT || env->legacy) {
        env->legacy = false;
        env->err = mi_option_parse(desc->option, value, &env->value);
      }
      return;
    }
    if (mi_env_name_eq(name, len, desc->legacy_name)) {
      if (env->err == ENOENT) {
        env->legacy = true;
        env->err = mi_option_parse(desc->option, value, &env->value);
      }
      return;
    }
  }
  if (!mi_env_is_extra(name, len)) {
    mi_env_scan_unknown(scan, entry, plen + len);
  }
}

// Initialize all pending options in a single pass over the environment.
// Returns `false` if the full environment cannot be iterated.
static bool mi_options_init_from_environ(void) {
  mi_env_scan_t scan;
  for (size_t i = 0; i < _mi_option_last; i++) {
    const mi_option_env_t not_found = MI_OPTION_ENV_NOT_FOUND;
    scan.options[i] = not_found;
  }
  scan.unknown_count = 0;
  scan.unknown[0] = 0;
  if (!_mi_env_visit(&mi_env_scan_entry, &scan)) return false;
  // first apply all options (so warnings use the final values for `verbose`, `show_errors` etc.)
  for (size_t i = 0; i < _mi_option_last; i++) {
    const mi_option_t option = (mi_option_t)i;
    mi_option_state_t* const state = mi_option_state(option);
    if (mi_option_state_init(state) == MI_OPTION_PENDING) {
      mi_option_env_apply(option, state, &scan.options[i]);
    }
    else {
      scan.options[i].err = ENOENT;   // already initialized (e.g. set explicitly)
      scan.options[i].legacy = false;
    }
  }
  // and then emit warnings
  for (size_t i = 0; i < _mi_option_last; i++) {
    mi_option_env_warn((mi_option_t)i, &scan.options[i]);
  }
  if (scan.unknown_count > 0 && !_mi_os_is_elevated()) {
    _mi_warning_message("unknown environment option(s) are ignored: %s\n", (scan.unknown[0] != 0 ? scan.unknown : "..."));
  }
  return true;
}
