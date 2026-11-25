/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_HEAP_HEAP_H
#define MI_HEAP_HEAP_H

#include "../types.h"
#include "process.h"
#include "subproc.h"
#include "../prim/prim-pac.h"

void          _mi_heap_init(mi_heap_t* heap, mi_thread_local_t theap, mi_subproc_t* subproc, mi_arena_id_t exclusive_arena_id, bool use_qsbr);
mi_decl_cold  mi_theap_t* _mi_heap_theap_get_or_init(const mi_heap_t* heap);  // get (and possible create) the theap belonging to a heap
void          _mi_heap_force_destroy(mi_heap_t* heap, bool acquire_heaps_lock); // allow destroying the main heap
mi_heap_t*    _mi_heap_new_for_subproc(mi_subproc_t* subproc, mi_arena_id_t exclusive_arena_id, bool is_heap_main, bool use_qsbr);
bool          _mi_heap_theap_set(mi_heap_t* heap, mi_theap_t* theap);

// Heap and theap handles passed to the API are validated by a cookie: a keyed MAC of the handle address
// using the random process key. The cookie is set at initialization and cleared when the (t)heap is freed
// such that a stale (deleted) or forged handle is detected (unless a new (t)heap reuses the same address).
// For heaps this is only checked on cold paths, as on the fast path a cached theap with `theap->heap == heap`
// already implies the handle is valid. For theaps it is checked on cold paths and in `_mi_malloc_generic`.
mi_decl_cold bool _mi_heap_invalid(const mi_heap_t* heap);     // reports EFAULT and returns false
mi_decl_cold bool _mi_theap_invalid(const mi_theap_t* theap);  // reports EFAULT and returns false

#define MI_COOKIE_TAG_HEAP   (0x4EA9)
#define MI_COOKIE_TAG_THEAP  (0x7EA9)

static inline mi_addr_t mi_handle_cookie(const void* p, size_t tag) {
  #if MI_PAC
  // already keyed (by `pacga` or the process key); passing the process key as the salt would cancel it in weak mode
  return mi_keyed_mac(mi_addr(p), mi_salt_of(tag));
  #else
  return mi_mul_xor64(mi_addr(p), mi_process_get_sealed_once()->process_key ^ mi_salt_of(tag));
  #endif
}

static inline mi_addr_t mi_heap_cookie(const mi_heap_t* heap) {
  return mi_handle_cookie(heap, MI_COOKIE_TAG_HEAP);
}

static inline bool mi_heap_handle_is_valid(const mi_heap_t* heap) {
  return ((heap!=NULL && heap->cookie == mi_heap_cookie(heap)) || _mi_heap_invalid(heap));
}

static inline mi_addr_t mi_theap_cookie(const mi_theap_t* theap) {
  return mi_handle_cookie(theap, MI_COOKIE_TAG_THEAP);
}

static inline bool mi_theap_has_valid_cookie(const mi_theap_t* theap) {
  return (theap->cookie == mi_theap_cookie(theap));
}

// Check an initialized theap (uninitialized (empty) theaps have no valid cookie)
static inline bool mi_theap_handle_is_valid(const mi_theap_t* theap) {
  return ((mi_theap_has_valid_cookie(theap)) || _mi_theap_invalid(theap));
}

static inline bool mi_heap_qsbr_is_enabled(const mi_heap_t* heap) {
#if MI_QSBR
  return heap->qsbr_enabled;
#else
  MI_UNUSED(heap);
  return false;
#endif
}

static inline long mi_heap_reclaim_on_free(mi_heap_t* heap) {
  return heap->reclaim_on_free;
}

static inline mi_heap_t* mi_heap_get_heap_main(const mi_heap_t* heap) {
  return _mi_subproc_heap_main(heap->subproc);
}

static inline bool mi_is_heap_main(const mi_heap_t* heap) {
  mi_assert_internal(heap!=NULL);
  return (mi_heap_get_heap_main(heap) == heap);
}

static inline bool mi_is_process_heap_main(const mi_heap_t* heap) {
  mi_assert_internal(heap!=NULL);
  return (_mi_subproc_main()->heap_main == heap);
}

#endif
