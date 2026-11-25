/* ----------------------------------------------------------------------------
Copyright (c) 2018-2023, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

// This file is included in `src/prim/prim.c`

#include "mimalloc.h"
#include "mimalloc/prim/prim.h"
#include "mimalloc/prim/os.h"
#include "mimalloc/util/libc.h"
#include "mimalloc/heap/process.h"

#include <stdio.h>   // fputs
#include <stdlib.h>  // getenv
#include <unistd.h>  // sbrk, sleep, getentropy

//---------------------------------------------
// Initialize
//---------------------------------------------

void _mi_prim_mem_init( mi_os_mem_config_t* config ) {
  config->page_size = 64*MI_KiB; // WebAssembly has a fixed page size: 64KiB
  config->alloc_granularity = 16;
  config->has_overcommit = false;
  config->has_partial_free = false;
  config->has_virtual_reserve = false;
  config->has_reset = false;
}

//---------------------------------------------
// Free
//---------------------------------------------

int _mi_prim_free(void* addr, size_t size ) {
  MI_UNUSED(addr); MI_UNUSED(size);
  // wasi theap cannot be shrunk
  return 0;
}


//---------------------------------------------
// Allocation: sbrk or memory_grow
//---------------------------------------------

#if defined(MI_USE_SBRK)
  static void* mi_memory_grow( size_t size ) {
    void* p = sbrk(size);
    if (p == (void*)(-1)) return NULL;
    #if !defined(__wasi__) // on wasi this is always zero initialized already (?)
    memset(p,0,size);
    #endif
    return p;
  }
#elif defined(__wasi__)
  static void* mi_memory_grow( size_t size ) {
    size_t base = (size > 0 ? __builtin_wasm_memory_grow(0,mi_divide_up(size, _mi_os_page_size()))
                            : __builtin_wasm_memory_size(0));
    if (base == SIZE_MAX) return NULL;
    return (void*)(base * _mi_os_page_size());
  }
#endif

#if MI_USE_PTHREADS
static pthread_mutex_t mi_theap_grow_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static void* mi_prim_mem_grow(size_t size, size_t try_alignment) {
  void* p = NULL;
  if (try_alignment <= 1) {
    // `sbrk` is not thread safe in general so try to protect it (we could skip this on WASM but leave it in for now)
    #if MI_USE_PTHREADS
    pthread_mutex_lock(&mi_theap_grow_mutex);
    #endif
    p = mi_memory_grow(size);
    #if MI_USE_PTHREADS
    pthread_mutex_unlock(&mi_theap_grow_mutex);
    #endif
  }
  else {
    void* base = NULL;
    size_t alloc_size = 0;
    // to allocate aligned use a lock to try to avoid thread interaction
    // between getting the current size and actual allocation
    // (also, `sbrk` is not thread safe in general)
    #if MI_USE_PTHREADS
    pthread_mutex_lock(&mi_theap_grow_mutex);
    #endif
    {
      void* current = mi_memory_grow(0);  // get current size
      if (current != NULL) {
        void* aligned_current = mi_align_up_ptr(current, try_alignment);  // and align from there to minimize wasted space
        alloc_size = mi_align_up( ((uint8_t*)aligned_current - (uint8_t*)current) + size, _mi_os_page_size());
        base = mi_memory_grow(alloc_size);
      }
    }
    #if MI_USE_PTHREADS
    pthread_mutex_unlock(&mi_theap_grow_mutex);
    #endif
    if (base != NULL) {
      p = mi_align_up_ptr(base, try_alignment);
      if ((uint8_t*)p + size > (uint8_t*)base + alloc_size) {
        // another thread used wasm_memory_grow/sbrk in-between and we do not have enough
        // space after alignment. Give up (and waste the space as we cannot shrink :-( )
        // (in `mi_os_mem_alloc_aligned` this will fall back to overallocation to align)
        p = NULL;
      }
    }
  }
  /*
  if (p == NULL) {
    _mi_warning_message("unable to allocate sbrk/wasm_memory_grow OS memory (%zu bytes, %zu alignment)\n", size, try_alignment);
    errno = ENOMEM;
    return NULL;
  }
  */
  mi_assert_internal( p == NULL || try_alignment == 0 || mi_addr(p) % try_alignment == 0 );
  return p;
}

// Note: the `try_alignment` is just a hint and the returned pointer is not guaranteed to be aligned.
int _mi_prim_alloc(void* hint_addr, size_t size, size_t try_alignment, bool commit, bool allow_large, bool* is_large, bool* is_zero, void** addr) {
  MI_UNUSED(allow_large); MI_UNUSED(commit); MI_UNUSED(hint_addr);
  *is_large = false;
  *addr = mi_prim_mem_grow(size, try_alignment);
  *is_zero = (*addr != NULL);  // memory.grow (and wasi-libc sbrk) always appends zero-initialized pages
  return (*addr != NULL ? 0 : ENOMEM);
}


//---------------------------------------------
// Commit/Reset/Protect
//---------------------------------------------

int _mi_prim_commit(void* addr, size_t size, bool* is_zero) {
  MI_UNUSED(addr); MI_UNUSED(size);
  *is_zero = false;
  return 0;
}

int _mi_prim_decommit(void* addr, size_t size, bool* needs_recommit) {
  MI_UNUSED(addr); MI_UNUSED(size);
  *needs_recommit = false;
  return 0;
}

int _mi_prim_reset(void* addr, size_t size, bool eager) {
  MI_UNUSED(addr); MI_UNUSED(size); MI_UNUSED(eager);
  return 0;
}

int _mi_prim_reuse(void* addr, size_t size) {
  MI_UNUSED(addr); MI_UNUSED(size);
  return 0;
}

int _mi_prim_guard(void* addr, size_t size, bool set_guard, bool is_committed) {
  MI_UNUSED(addr); MI_UNUSED(size); MI_UNUSED(set_guard); MI_UNUSED(is_committed);
  return 0;
}

int _mi_prim_protect(void* addr, size_t size, mi_protect_t protect) {
  MI_UNUSED(addr); MI_UNUSED(size); MI_UNUSED(protect);
  return 0;
}


//---------------------------------------------
// Huge pages and NUMA nodes
//---------------------------------------------

int _mi_prim_alloc_huge_os_pages(void* hint_addr, size_t size, int numa_node, bool* is_zero, void** addr) {
  MI_UNUSED(hint_addr); MI_UNUSED(size); MI_UNUSED(numa_node);
  *is_zero = true;
  *addr = NULL;
  return ENOSYS;
}

size_t _mi_prim_numa_node(void) {
  return 0;
}

size_t _mi_prim_numa_node_count(void) {
  return 1;
}


//----------------------------------------------------------------
// Clock
//----------------------------------------------------------------

#include <time.h>

// low resolution timer
static mi_msecs_t mi_prim_clock_now_lowres(void) {
  #if !defined(_WASI_EMULATED_PROCESS_CLOCKS)
  return 0;  // `clock` requires -D_WASI_EMULATED_PROCESS_CLOCKS -lwasi-emulated-process-clocks
  #else
  const int64_t ticks = (int64_t)clock();
  #if !defined(CLOCKS_PER_SEC)
    return ticks;
  #else
    if (CLOCKS_PER_SEC <= 0 || CLOCKS_PER_SEC == 1000) {
      return ticks;
    }
    else if (CLOCKS_PER_SEC > 0 && CLOCKS_PER_SEC < 1000) {
      return ticks * (1000 / (mi_msecs_t)CLOCKS_PER_SEC);
    }
    else {
      return ticks / ((mi_msecs_t)CLOCKS_PER_SEC / 1000);
    }
  #endif
  #endif
}

mi_msecs_t _mi_prim_clock_now(void) {
  #if defined(CLOCK_REALTIME) || defined(CLOCK_MONOTONIC)
    #ifdef CLOCK_MONOTONIC
    const clockid_t clockid = CLOCK_MONOTONIC;
    #else
    const clockid_t clockid = CLOCK_REALTIME;
    #endif
    struct timespec t;
    if (clock_gettime(clockid,&t) == 0) {
      return ((mi_msecs_t)t.tv_sec * 1000) + ((mi_msecs_t)t.tv_nsec / 1000000L);
    }
  #endif
  return mi_prim_clock_now_lowres();
}


//----------------------------------------------------------------
// Process info
//----------------------------------------------------------------

void _mi_prim_process_info(mi_process_info_t* pinfo)
{
  // use defaults
  MI_UNUSED(pinfo);
}


//----------------------------------------------------------------
// Output
//----------------------------------------------------------------

void _mi_prim_out_stderr( const char* msg ) {
  fputs(msg,stderr);
}


//----------------------------------------------------------------
// Environment
//----------------------------------------------------------------

#include <wasi/api.h>  // __wasi_environ_sizes_get, __wasi_environ_get

// Read the environment directly as `getenv` allocates on first use (which can recurse into the allocator).
// A small environment is read into a stack buffer, while a large one is read once into memory from `memory.grow`.
#define MI_WASI_ENV_BUF_SIZE  (4*MI_KiB)
static uint8_t** mi_wasi_env_large;  // = NULL

bool _mi_prim_env_visit(mi_env_visit_fun* visit, void* arg) {
  __wasi_size_t count = 0;
  __wasi_size_t size  = 0;
  if (__wasi_environ_sizes_get(&count, &size) != 0) return false;
  uint8_t* buf[MI_WASI_ENV_BUF_SIZE / sizeof(uint8_t*)];
  uint8_t** env = buf;
  const size_t env_size = (count + 1)*sizeof(uint8_t*) + size;
  if (env_size <= sizeof(buf)) {
    if (__wasi_environ_get(env, (uint8_t*)&env[count + 1]) != 0) return false;
  }
  else if (mi_wasi_env_large != NULL) {
    env = mi_wasi_env_large;
  }
  else {
    env = (uint8_t**)mi_prim_mem_grow(mi_align_up(env_size, 64*MI_KiB), 0);  // the WASI environment does not change
    if (env == NULL || __wasi_environ_get(env, (uint8_t*)&env[count + 1]) != 0) return false;
    mi_wasi_env_large = env;
  }
  for (__wasi_size_t i = 0; i < count; i++) {
    visit((const char*)env[i], arg);
  }
  return true;
}

int _mi_prim_getenv(const char* name, char* result, size_t result_size) {
  return _mi_prim_getenv_by_visit(name, result, result_size);
}


//----------------------------------------------------------------
// Random
//----------------------------------------------------------------

bool _mi_prim_random_buf(void* buf, size_t buf_len) {
  return (getentropy(buf, buf_len) == 0);  // uses `random_get` (and buf_len <= 256)
}


//----------------------------------------------------------------
// Thread init/done
//----------------------------------------------------------------

void _mi_prim_thread_init_auto_done(void) {
  // nothing
}

void _mi_prim_thread_done_auto_done(void) {
  // nothing
}

void _mi_prim_thread_associate_default_theap(mi_theap_t* theap) {
  MI_UNUSED(theap);
}

void _mi_prim_thread_init(void) {
  // nothing
}

bool _mi_prim_thread_is_in_threadpool(void) {
  return false;
}

void _mi_prim_thread_yield(void) {
  sleep(0);
}

bool _mi_prim_process_is_killed(void) {
  return false;
}
