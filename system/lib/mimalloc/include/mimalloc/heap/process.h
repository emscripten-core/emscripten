/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_HEAP_PROCESS_H
#define MI_HEAP_PROCESS_H

#include "../types.h"
#include "../prim/prim.h"          // mi_os_mem_config_t
#include "../util/options.h"
#include "../util/libc.h"     // mi_align_down_ptr

#if MI_MTE
#include "../prim/prim-mte.h"  // mi_memtag_load
#endif

mi_heap_t*    _mi_subproc_heap_main(mi_subproc_t* subproc);
mi_theap_t*   _mi_theap_main(void);  // current thread's backing theap, without initialization
mi_page_t*    _mi_page_empty_get(void);
void          _mi_auto_process_init(void);
void mi_cdecl _mi_auto_process_done(void) mi_attr_noexcept;
bool          _mi_preloading(void);           // true while the C runtime is not initialized yet
void          _mi_thread_done(mi_theap_t* theap);
mi_theap_t*   _mi_thread_init(void);
mi_theap_t*   _mi_thread_init_with_heap(mi_heap_t* heap);
bool          _mi_is_empty_theap(const mi_theap_t* theap);
mi_theap_t*   _mi_theap_unavailable(void);    // empty default theap of a thread after `mi_thread_done`, or returned if a theap could not be created
bool          _mi_thread_is_done(void);       // true after `mi_thread_done` (until an explicit `mi_thread_init`)

extern mi_decl_hidden const mi_theap_t _mi_theap_empty; // read-only empty theap, initial value of the thread local default theap (in the MI_TLS_MODEL_LOCAL)

// Fork state
typedef struct mi_fork_state_s {
  size_t              epoch;                    // incremented in the child at every fork
  _Atomic(size_t)     pending;                  // set in the child until the dead theaps are abandoned
  bool                torn;                     // set in a child that may have a torn snapshot of memory (on macOS)
  mi_heap_t*          suspended_heap;           // heap with suspended abandoned reclaim (on macOS)
  size_t              suspended_count[MI_BIN_COUNT];  // and its abandoned counts
} mi_fork_state_t;

// OS state (see `prim/os.c` and the `prim/<os>/prim.c` files)
typedef struct mi_os_state_s {
  _Atomic(size_t)     numa_node_count;          // cached NUMA node count (0 until first use)
  _Atomic(mi_addr_t)  aligned_base;             // next aligned address hint (see `_mi_os_get_aligned_hint`)
  _Atomic(mi_addr_t)  huge_start;               // next address for huge OS pages (see `mi_os_claim_huge_pages`)
  _Atomic(size_t)     large_page_access;        // 0: unknown, 1: granted, 2: denied (e.g. needs `SeLockMemoryPrivilege` on Windows)
  _Atomic(size_t)     large_page_backoff;       // skip large OS page allocation while > 0 (set after a failed attempt)
  _Atomic(size_t)     huge_page_unavailable;    // set once a huge (1GiB) OS page allocation failed
  _Atomic(size_t)     lazy_reset_unavailable;   // set if a lazy reset (`MADV_FREE`) is not supported (and we use `MADV_DONTNEED`)
} mi_os_state_t;


// Process meta data: this is allocated from the OS at process initialization (see `heap/process.c:mi_process_metadata_init`)
// instead of being static so it is not at a fixed offset from the program image, is surrounded by guard pages in secure mode,
// and can be memory tagged. With memory tagging, each field has its own tag (where adjacent fields have different tags),
// so fields are aligned with `mi_decl_cache_align` and accessed through `mi_process_metadata_field` (which loads the tag).
// Apply `F(tp,name)` to each field of the process meta data (used for the definition and for tagging each field)
#define mi_process_metadata_fields(F) \
  F(mi_subproc_t,     subproc_main)             /* initial subprocess */ \
  F(mi_heap_t,        heap_main)                /* initial heap for the process */ \
  F(mi_theap_t,       theap_main)               /* initial theap for the main thread */ \
  F(mi_tld_t,         tld_main)                 /* initial tld for the main thread */ \
  F(mi_theap_t,       theap_meta)               /* detached theap for meta-data allocation */ \
  F(mi_tld_t,         tld_meta)                 /* detached tld for `theap_meta` */ \
  F(mi_theap_t,       theap_unavailable)        /* empty theap used for a thread after `mi_thread_done`, or if a theap for a heap could not be allocated */ \
  F(mi_memid_t,       memid)                    /* OS provenance of the meta data (never freed) */

#define mi_process_metadata_field_decl(tp,name)  mi_decl_cache_align tp name;

typedef struct mi_process_metadata_s {
  mi_process_metadata_fields(mi_process_metadata_field_decl)
} mi_process_metadata_t;

// Sealed process data that is only written during process initialization (see `heap/process.c:_mi_process_unseal_once`).
// In secure mode it is read-only after initialization (and immutable if the OS supports it, see `MI_PROTECT_READ_SEALED`).
typedef struct mi_process_sealed_once_s {
  _Atomic(struct mi_page_map_s*)  page_map;     // the page map without memory tag (`NULL` before initialization) (see `heap/page-map.h:mi_page_map_untagged`); must be the first entry for better code-gen in secure mode
  mi_addr_t           process_key;              // random process key for encoded pointers (see `prim/prim-pac.h`) and heap cookies (see `heap/heap.h:mi_heap_cookie`); next to `page_map` as it is used for page map lookups
  mi_process_metadata_t* metadata;              // process meta data without memory tag (`NULL` before initialization) (see `mi_process_metadata`)
  mi_os_mem_config_t  os_mem_config;            // OS memory configuration (see `prim/os.c`)
  mi_msecs_t          clock_process_start;      // clock at process initialization (see `util/stats.c`)
  mi_msecs_t          clock_overhead;           // measured overhead of reading the clock (subtracted in `_mi_clock_end`)
  int64_t             clock_ticks_per_msec;     // performance counter ticks per millisecond (on Windows)
  bool                is_sealed;                // `true` after process initialization (and no further writes are allowed)
  bool                is_protected;             // `true` if the sealed pages are read-only (see `_mi_process_unseal`)
} mi_process_sealed_once_t;

// Sealed process data that can be updated at runtime (see `heap/process.c:_mi_process_unseal` for writing).
// In secure mode it is read-only except during a (locked) update.
typedef struct mi_process_sealed_s {
  _Atomic(size_t)     page_map_committed;       // committed entries in the page map

  // user callbacks (see `_mi_process_set_callback`)
  _Atomic(size_t)     callbacks_seq;            // sequence number for the callbacks (odd while updating)
  _Atomic(void*)      deferred_free;            // is `mi_deferred_free_fun*` (but some platforms don't support atomic function pointers)
  _Atomic(void*)      deferred_arg;             // argument to the deferred free function
  _Atomic(void*)      out_default;              // is `mi_output_fun*`
  _Atomic(void*)      out_arg;                  // argument to the output function
  _Atomic(void*)      error_handler;            // is `mi_error_fun*`
  _Atomic(void*)      error_arg;                // argument to the error function

  mi_subproc_t*       subprocs;                 // list of all sub-processes without memory tag (written under `_mi_process.subprocs_lock`)
  mi_bitmap_t*        thread_locals_free;       // bitmap of free thread local slots (1=free, 0=in-use) without memory tag (written under `_mi_process.thread_locals_lock`)
  mi_memid_t          thread_locals_memid;      // provenance of `thread_locals_free` (a malloc base is stored without memory tag)
  mi_profiler_t*      profile_env_profiler;     // the pprof profiler if `MIMALLOC_PROFILE` was set, without memory tag (see `profile/pprof.c`)
} mi_process_sealed_t;


// On CHERI we cannot modify access to the .bss section
#if MI_CHERI && !defined(MI_SECURE_NO_STATIC_SEALED)
#define MI_SECURE_NO_STATIC_SEALED (1)
#endif
#if MI_CHERI && !defined(MI_SECURE_NO_STATIC_GUARD)
#define MI_SECURE_NO_STATIC_GUARD (1)
#endif

// In secure mode, the front area of the static process data ends with two read-only pages that contain
// `mi_process_sealed_once_t` and `mi_process_sealed_t` respectively. These are preceded by a guard page, and the
// process data is followed by a guard page (with guard areas of twice the maximal OS page size to fit them).
// With `MI_SECURE_NO_STATIC_GUARD` only the guard pages are omitted (as some garbage collectors scan the static data),
// but the sealed pages are still read-only.
#if MI_SECURE && MI_OS_HAS_GUARD_PAGES && !MI_SECURE_NO_STATIC_SEALED
  #define MI_SECURE_STATIC_SEALED     (1)
#else
  #define MI_SECURE_STATIC_SEALED     (0)
#endif
#if MI_SECURE_STATIC_SEALED && !MI_SECURE_NO_STATIC_GUARD
  #define MI_SECURE_STATIC_GUARD    (2*MI_OS_PROBABLE_PAGE_SIZE)  // checked at runtime
#else
  #define MI_SECURE_STATIC_GUARD    (MI_CACHE_LINE_SIZE)          // no guard pages; just a cache line
#endif

// With sealed pages, we page align the process data (if the toolchain and loader support it) such that the
// address of the sealed pages are link-time constants and `mi_process_get_sealed_once` is fast (as used on the free path).
// (Windows toolchains cannot align beyond 8 KiB, and older ELF loaders may not honor alignments beyond the OS page size)
#if MI_SECURE_STATIC_SEALED && defined(__GNUC__) && !defined(_WIN32) && (MI_OS_PROBABLE_PAGE_SIZE <= 16*MI_KiB)
  #define MI_SECURE_SEALED_ALIGNED  (1)
  #define mi_decl_process_align     mi_decl_align(MI_OS_PROBABLE_PAGE_SIZE)
#else
  #define MI_SECURE_SEALED_ALIGNED  (0)
  #define mi_decl_process_align     mi_decl_cache_align
#endif

// The size of the front area (that contains the sealed pages in secure mode)
#if !MI_SECURE_STATIC_SEALED
  #define MI_SECURE_SEALED_GUARD    (MI_SECURE_STATIC_GUARD)
#elif !MI_SECURE_NO_STATIC_GUARD
  #define MI_SECURE_SEALED_GUARD    (MI_SECURE_STATIC_GUARD + 2*MI_OS_PROBABLE_PAGE_SIZE)
#elif MI_SECURE_SEALED_ALIGNED
  #define MI_SECURE_SEALED_GUARD    (2*MI_OS_PROBABLE_PAGE_SIZE)
#else
  #define MI_SECURE_SEALED_GUARD    (3*MI_OS_PROBABLE_PAGE_SIZE)  // with room to align the sealed pages
#endif

#if MI_SECURE_SEALED_ALIGNED && (MI_SECURE_SEALED_GUARD % MI_OS_PROBABLE_PAGE_SIZE) != 0
#error "the sealed pages must be page aligned"
#endif

// Output before an output function is registered (or the C runtime is initialized) is buffered (see `util/options.c`)
#ifndef MI_MAX_DELAY_OUTPUT
#define MI_MAX_DELAY_OUTPUT         (16*MI_KiB)
#endif

// Statically allocated process data. This is zero initialized and padded such that
// in secure mode it is surrounded by guard pages (see `heap/process.c:mi_process_guard_init`)
typedef struct mi_process_s {
  uint8_t             guard_front[MI_SECURE_SEALED_GUARD];  // in secure mode, contains the sealed pages (see `mi_process_get_sealed_once`)
  #if !MI_SECURE_STATIC_SEALED
  mi_process_sealed_once_t sealed_once;         // sealed data (unprotected)
  mi_process_sealed_t sealed;
  #endif

  bool                process_is_initialized;   // set to `true` in `mi_process_init`
  bool                libc_is_initialized;      // `true` once the C runtime is initialized (and `false` again after process done)
  bool                malloc_is_redirected;     // `true` if malloc is redirected to mimalloc (on Windows with `mimalloc-redirect.dll`)
  _Atomic(size_t)     process_done_is_called;   // set once `mi_process_done` is called
  _Atomic(size_t)     subprocs_created_count;   // total number of sub-processes created (used for `subproc_seq`)
  _Atomic(size_t)     error_count;              // stop emitting errors when >= `mi_option_max_errors`
  _Atomic(size_t)     warning_count;            // stop emitting warnings when >= `mi_option_max_warnings`
  _Atomic(size_t)     output_buf_len;           // used length of `output_buf`
  _Atomic(size_t)     profiler_generation;      // fresh generation for each profiler attach (see `profile/sample-profile.c`)

  mi_lock_t           thread_locals_lock;       // initialized in `_mi_thread_locals_init` (see `util/threadlocal.c`)
  mi_thread_local_t   thread_locals_version;    // version to be able to reuse slots safely

  mi_os_state_t       os;                       // OS state
  mi_fork_state_t     fork;                     // fork state (see `heap/subproc.c`)
  mi_lock_t           subprocs_lock;            // initialized in `_mi_subproc_main_init` (as a zero lock is invalid on some platforms)
  mi_atomic_guard_t   profile_snapshot_guard;   // allow only one thread at a time to take a profile snapshot
  mi_atomic_guard_t   arena_purge_guard;        // allow only one thread at a time to purge arenas

  // arrays last
  mi_option_state_t   options[_mi_option_last]; // current option values (see `util/options.c`)
  char                output_buf[MI_MAX_DELAY_OUTPUT+1]; // delayed output buffer (see `util/options.c`)

  // end guard
  mi_decl_cache_align uint8_t guard_end[MI_SECURE_STATIC_GUARD];
} mi_process_t;

extern mi_decl_hidden mi_decl_process_align mi_process_t _mi_process;

// Get the sealed-once process data (for reading only, see `_mi_process_unseal_once` for writing).
// In secure mode, this is the second to last full OS page within `guard_front`.
static inline const mi_process_sealed_once_t* mi_process_get_sealed_once(void) {
  #if MI_SECURE_SEALED_ALIGNED
  return (const mi_process_sealed_once_t*)((uint8_t*)&_mi_process + MI_SECURE_SEALED_GUARD - 2*MI_OS_PROBABLE_PAGE_SIZE);
  #elif MI_SECURE_STATIC_SEALED
  return (const mi_process_sealed_once_t*)mi_align_down_ptr((uint8_t*)&_mi_process + MI_SECURE_SEALED_GUARD - 2*MI_OS_PROBABLE_PAGE_SIZE, MI_OS_PROBABLE_PAGE_SIZE);
  #else
  return &_mi_process.sealed_once;
  #endif
}

// Get the sealed process data (for reading only, see `_mi_process_unseal` for writing).
// In secure mode, this is the OS page just after the sealed-once page (and just before the page where the process data starts).
static inline const mi_process_sealed_t* mi_process_get_sealed(void) {
  #if MI_SECURE_STATIC_SEALED
  return (const mi_process_sealed_t*)((uint8_t*)mi_process_get_sealed_once() + MI_OS_PROBABLE_PAGE_SIZE);
  #else
  return &_mi_process.sealed;
  #endif
}

// With memory tagging, load the memory tag of a (non-NULL) pointer `p` to meta data.
// This is used for meta data pointers that are derived from an address, or that are stored
// without their tag (like the page map and process meta data pointers in the sealed process data,
// so these cannot be leaked by reading the (untagged) static process data).
static inline void* mi_meta_load(void* p) {
  mi_assert_internal(p != NULL);
  #if MI_MTE >= 2
  return mi_memtag_load(p);  // the CPU supports MTE and tagging is enabled at runtime (`-DMI_MTE=ON`)
  #else
  #if MI_MTE
  if (mi_process_get_sealed_once()->os_mem_config.has_memtag) { return mi_memtag_load(p); }
  #endif
  return p;
  #endif
}

// As `mi_meta_load` but `p` can be NULL.
static inline void* mi_meta_load_maybe(void* p) {
  return (p == NULL ? NULL : mi_meta_load(p));
}

mi_process_metadata_t* _mi_process_metadata_init(void);

// Get the process meta data without memory tag (allocated at process initialization, or lazily on first access).
// Do not access fields through this pointer directly but use `mi_process_metadata_field` instead.
static inline mi_process_metadata_t* mi_process_metadata(void) {
  mi_process_metadata_t* const metadata = mi_process_get_sealed_once()->metadata;  // stored without memory tag
  return (metadata != NULL ? metadata : _mi_process_metadata_init());
}

// Get a pointer (with its memory tag) to a field of the process meta data.
#define mi_process_metadata_field(tp,field)   ((tp*)mi_meta_load(&mi_process_metadata()->field))

// Is this the unavailable theap? (the default theap of a thread after `mi_thread_done`, or returned if a theap could not be created)
static inline bool _mi_theap_is_unavailable(const mi_theap_t* theap) {
  return (mi_addr(theap) == mi_addr(&mi_process_metadata()->theap_unavailable));
}

// Make the sealed process data writable (and take a lock); must be followed by `_mi_process_seal`.
mi_process_sealed_t* _mi_process_unseal(void);
void                 _mi_process_seal(void);

// Set or get a callback function and argument pair in the sealed process data (as a consistent pair).
void  _mi_process_set_callback(const _Atomic(void*)* pfun, const _Atomic(void*)* parg, void* fun, void* arg);
void* _mi_process_get_callback(const _Atomic(void*)* pfun, const _Atomic(void*)* parg, void** arg);

// Write the sealed-once process data (and take a lock); must be followed by `_mi_process_seal_once`.
// This is only allowed during process initialization (and is a fatal error afterwards).
mi_process_sealed_once_t* _mi_process_unseal_once(void);
void                      _mi_process_seal_once(void);

#endif
