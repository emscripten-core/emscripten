/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_PRIM_H
#define MI_PRIM_H

#include "../types.h"

// --------------------------------------------------------------------------
// This file specifies the primitive portability API.
// Each OS/host needs to implement these primitives, see `src/prim`
// for implementations on Window, macOS, WASI, and Linux/Unix.
//
// note: on all primitive functions, we always have result parameters != NULL, and:
//  addr != NULL and page aligned
//  size > 0     and page aligned
//  the return value is an error code as an `int` where 0 is success
// --------------------------------------------------------------------------

// OS memory configuration
typedef struct mi_os_mem_config_s {
  size_t  page_size;              // default to 4KiB
  size_t  large_page_size;        // 0 if not supported, usually 2MiB
  size_t  alloc_granularity;      // smallest allocation size (usually 4KiB, on Windows 64KiB)
  size_t  physical_memory_in_kib; // physical memory size in KiB
  size_t  virtual_address_bits;   // usually 48 or 56 bits on 64-bit systems. (used to determine secure randomization)
  size_t  memtag_mode;            // OS specific memory tag check mode (only used in the `prim` layer, see `unix/prim.c`)
  size_t  memtag_line_size;       // cache line size tagged at once by the CPU (`dc gva`); set in `_mi_os_init`
  bool    has_overcommit;         // can we reserve more memory than can be actually committed?
  bool    has_partial_free;       // can allocated blocks be freed partially? (true for mmap, false for VirtualAlloc)
  bool    has_virtual_reserve;    // supports virtual address space reservation? (if true we can reserve virtual address space without using commit or physical memory)
  bool    has_reset;              // can memory be reset? (if false, `_mi_prim_reset` is a no-op, e.g. on WebAssembly)
  bool    has_aligned_alloc;      // can the OS directly allocate at a requested alignment? (e.g. `VirtualAlloc2`, or `mmap` with `MAP_ALIGNED`)
  bool    has_transparent_huge_pages;  // true if transparent huge pages are enabled (on Linux)
  bool    is_elevated;            // running with elevated privileges (e.g. setuid)? (set in `_mi_os_init`)
  bool    has_pac;                // true if generic pointer authentication (`pacga`) is supported (only detected with `MI_PAC=ON`)
  bool    has_memtag;             // true if memory tagging (MTE) is enabled; all non-large OS memory then supports tagging
} mi_os_mem_config_t;

// Initialize
void _mi_prim_mem_init( mi_os_mem_config_t* config );

// Free OS memory
int _mi_prim_free(void* addr, size_t size );

// Allocate OS memory. Return NULL on error.
// The `try_alignment` is just a hint and the returned pointer does not have to be aligned.
// If `commit` is false, the virtual memory range only needs to be reserved (with no access)
// which will later be committed explicitly using `_mi_prim_commit`.
// `is_zero` is set to true if the memory was zero initialized (as on most OS's)
// The `hint_addr` address is either `NULL` or a preferred allocation address but can be ignored.
// pre: !commit => !allow_large
//      try_alignment >= _mi_os_page_size() and a power of 2
int _mi_prim_alloc(void* hint_addr, size_t size, size_t try_alignment, bool commit, bool allow_large, bool* is_large, bool* is_zero, void** addr);

// Commit memory. Returns error code or 0 on success.
// For example, on Linux this would make the memory PROT_READ|PROT_WRITE.
// `is_zero` is set to true if the memory was zero initialized (e.g. on Windows)
int _mi_prim_commit(void* addr, size_t size, bool* is_zero);

// Decommit memory. Returns error code or 0 on success. The `needs_recommit` result is true
// if the memory would need to be re-committed. For example, on Windows this is always true,
// but on Linux we could use MADV_DONTNEED to decommit which does not need a recommit.
// pre: needs_recommit != NULL
int _mi_prim_decommit(void* addr, size_t size, bool* needs_recommit);

// Reset memory. The range keeps being accessible but the content might be reset to zero at any moment.
// If `eager`, the memory should be released immediately (e.g. `MADV_DONTNEED` instead of `MADV_FREE`).
// Returns error code or 0 on success.
int _mi_prim_reset(void* addr, size_t size, bool eager);

// Reuse memory. This is called for memory that is already committed but
// may have been reset (`_mi_prim_reset`) or decommitted (`_mi_prim_decommit`) where `needs_recommit` was false.
// Returns error code or 0 on success. On most platforms this is a no-op.
int _mi_prim_reuse(void* addr, size_t size);

// Set a guard on a range making it inaccessible (`set_guard`), or remove it. Returns error code or 0 on success.
// The guard should not change the accounted commit state `is_committed` of the range:
// - if committed, a set keeps it committed (but the contents may be discarded), and a remove makes it accessible again.
// - otherwise (the range may be partly committed), a set makes it inaccessible (e.g. by decommitting on Windows)
//   and a remove is a no-op (it stays inaccessible until `_mi_prim_commit`).
int _mi_prim_guard(void* addr, size_t size, bool set_guard, bool is_committed);

// Memory access protection
typedef enum mi_protect_e {
  MI_PROTECT_NONE,       // inaccessible
  MI_PROTECT_READ,       // read-only
  MI_PROTECT_READWRITE,  // readable and writable
  MI_PROTECT_READ_SEALED // read-only, and if the OS supports it, immutable (the protection can no longer be changed)
} mi_protect_t;

// Set the access protection of a committed range. Returns error code or 0 on success
// (and on platforms that have no memory protection, return 0 as well).
// With `MI_PROTECT_READ_SEALED`, sealing is best effort (and only failing to make the range read-only is an error).
int _mi_prim_protect(void* addr, size_t size, mi_protect_t protect);

// Allocate huge (1GiB) pages possibly associated with a NUMA node.
// `is_zero` is set to true if the memory was zero initialized (as on most OS's)
// pre: size > 0  and a multiple of 1GiB.
//      numa_node is either negative (don't care), or a numa node number.
int _mi_prim_alloc_huge_os_pages(void* hint_addr, size_t size, int numa_node, bool* is_zero, void** addr);

// Return the current NUMA node
size_t _mi_prim_numa_node(void);

// Return the number of logical NUMA nodes
size_t _mi_prim_numa_node_count(void);

// Clock ticks
mi_msecs_t _mi_prim_clock_now(void);

// Return process information (only for statistics)
typedef struct mi_process_info_s {
  mi_msecs_t  elapsed;
  mi_msecs_t  utime;
  mi_msecs_t  stime;
  size_t      current_rss;
  size_t      peak_rss;
  size_t      current_commit;
  size_t      peak_commit;
  size_t      page_faults;
} mi_process_info_t;

void _mi_prim_process_info(mi_process_info_t* pinfo);

// Default stderr output. (only for warnings etc. with verbose enabled)
// msg != NULL && _mi_strlen(msg) > 0
void _mi_prim_out_stderr( const char* msg );

// Get an environment variable. (only for options)
// name != NULL, result != NULL, result_size >= 64
// Return 1 for success, 0 if not found, -2 if the value does not fit in `result`,
// and -1 on error (for example, if `getenv` cannot be called yet during preloading).
int _mi_prim_getenv(const char* name, char* result, size_t result_size);

// Visit all entries of the environment (only for options). Calls `visit(entry,arg)` for each
// zero terminated `name=value` entry (which is only valid during the call). The environment may be locked
// during the visit so `visit` should not call into the OS. Very long entries may be skipped.
// Returns `false` if the platform cannot iterate the full environment (or not yet, e.g. during preloading);
// in that case options are looked up one at a time using `_mi_prim_getenv`.
// visit != NULL
typedef void (mi_env_visit_fun)(const char* entry, void* arg);
bool _mi_prim_env_visit(mi_env_visit_fun* visit, void* arg);

// Generic implementation of `_mi_prim_getenv` using `_mi_prim_env_visit` (case insensitive).
// Can be used on platforms where visiting the environment does not allocate (e.g. using `environ`).
int _mi_prim_getenv_by_visit(const char* name, char* result, size_t result_size);


// Fill a buffer with strong randomness; return `false` on error or if
// there is no strong randomization available.
bool _mi_prim_random_buf(void* buf, size_t buf_len);

// Called on the first thread start, and should ensure `_mi_thread_done` is called on thread termination.
void _mi_prim_thread_init_auto_done(void);

// Called on process exit and may take action to clean up resources associated with the thread auto done.
void _mi_prim_thread_done_auto_done(void);

// Called when the default theap for a thread changes; the (opaque) `theap` value is passed
// to `_mi_thread_done` at thread termination (and is signed with `MI_PAC`).
void _mi_prim_thread_associate_default_theap(mi_theap_t* theap);

// Called once on each thread initialization (before the first allocation in that thread)
void _mi_prim_thread_init(void);

// Is this thread part of a thread pool?
bool _mi_prim_thread_is_in_threadpool(void);

// Yield to other threads. Should be similar to `sleep(0)`.
// Is called only in rare situations and does not have to be lightning fast.
void _mi_prim_thread_yield(void);

// Return `true` if the process is forcefully shutdown (with potentially killed threads that might still hold a lock)
// If this returns true, mimalloc skips graceful shutdown (as locks might still be held by threads that are killed).
// See also issue #1377
bool _mi_prim_process_is_killed(void);


// The following are predefined by default

// Is the mimalloc-redirect running (Windows only)
bool _mi_is_redirected(void);

// Called to initialize the allocator
bool _mi_allocator_init(const char** message);

// Called to clean up the allocator
void _mi_allocator_done(void);


#endif  // MI_PRIM_H
