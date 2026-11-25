/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/prim/prim.h"
#include "mimalloc/prim/prim-tls.h"
#include "mimalloc/util/options.h"              // _mi_error_message

// --------------------------------------------------------------------------
// Implement fast access to the thread local storage for `_mi_theap_default()`
// and `mi_theap_cached()`. See `include/mimalloc/prim/prim-tls.h` for more info
// on the TLS models.
// --------------------------------------------------------------------------

void _mi_tls_slots_init(void);
void _mi_tls_slots_done(void);
void _mi_theap_default_set(mi_theap_t* theap);
void _mi_theap_cached_set(mi_theap_t* theap);
void _mi_theap_tls_check(void);


#if MI_TLS_MODEL_LOCAL
// with `MI_PAC` the initial value is `NULL` (as we cannot statically sign the empty theap)
#if MI_PAC
#define MI_THEAP_LOCAL_INIT   NULL
#else
#define MI_THEAP_LOCAL_INIT   ((mi_theap_t*)&_mi_theap_empty)
#endif
// the thread-local main theap for allocation
mi_decl_hidden mi_decl_thread mi_theap_t* __mi_theap_default = MI_THEAP_LOCAL_INIT;
// the last used non-main theap
mi_decl_hidden mi_decl_thread mi_theap_t* __mi_theap_cached = MI_THEAP_LOCAL_INIT;
#endif

mi_decl_hidden mi_decl_thread void* __mi_thread_id_helper = NULL;

mi_thread_id_t _mi_thread_id(void) mi_attr_noexcept {
  const mi_thread_id_t tid = _mi_prim_thread_id();
  mi_assert_internal( (tid & MI_PAGE_FLAG_MASK) == 0 ); // mimalloc reserves the bottom 2 bits
  return tid;
}

// ----------------------------------------------------------------------------
// Setting the default and cached theap
// ----------------------------------------------------------------------------

#if MI_TLS_MODEL_WIN32

// If we can, we use one of the 64 direct TLS slots (but fall back to expansion slots if needed)
// See <https://en.wikipedia.org/wiki/Win32_Thread_Information_Block> for the offsets.
#if MI_SIZE_SIZE==4
#define MI_TLS_DIRECT_FIRST             (0x0E10 / MI_SIZE_SIZE)
#else
#define MI_TLS_DIRECT_FIRST             (0x1480 / MI_SIZE_SIZE)
#endif
#define MI_TLS_DIRECT_SLOTS             (64)
#define MI_TLS_EXPANSION_SLOTS          (1024)

#if !MI_WIN_DIRECT_TLS
// We initially use the last of the expansion slots as the default NULL.
// note: this will fail if the program allocates exactly 1024+64 slots with TlsAlloc
// before we are initialized :-( (but this seems quite unlikely).
// (todo: another approach could be to use slot 7 (EnvironmentPointer) as the initial slot as that seems to be always NULL)
#define MI_TLS_INITIAL_SLOT             MI_TLS_EXPANSION_SLOT
#define MI_TLS_INITIAL_EXPANSION_SLOT   (MI_TLS_EXPANSION_SLOTS-1)
#else
// With direct tls we need an initial NULL slot outside the expansion slots
#define MI_TLS_INITIAL_SLOT             (5)  // Arbitrary user pointer
#define MI_TLS_INITIAL_EXPANSION_SLOT   (MI_TLS_EXPANSION_SLOTS-1)  // unused
#endif

// in case of errors assign fixed slots (but since we use EFAULT the program should fail anyways)
#define MI_TLS_ERROR_SLOT               (5)   // arbitrary user pointer
#define MI_TLS_ERROR_EXPANSION_SLOT     (7)   // environment pointer (only used for OS/2 emulation)


mi_decl_hidden mi_decl_cache_align _Atomic(size_t) _mi_theap_default_slot = MI_ATOMIC_VAR_INIT(MI_TLS_INITIAL_SLOT);
mi_decl_hidden _Atomic(size_t) _mi_theap_default_expansion_slot = MI_ATOMIC_VAR_INIT(MI_TLS_INITIAL_EXPANSION_SLOT);
mi_decl_hidden _Atomic(size_t) _mi_theap_cached_slot            = MI_ATOMIC_VAR_INIT(MI_TLS_INITIAL_SLOT);
mi_decl_hidden _Atomic(size_t) _mi_theap_cached_expansion_slot  = MI_ATOMIC_VAR_INIT(MI_TLS_INITIAL_EXPANSION_SLOT);

static DWORD mi_tls_raw_index_default = TLS_OUT_OF_INDEXES;
static DWORD mi_tls_raw_index_cached  = TLS_OUT_OF_INDEXES;

static bool mi_win_tls_slot_alloc(_Atomic(size_t)* slot, _Atomic(size_t)* extended, DWORD* raw_index) {
  // always write slot before extended due to concurrent readers
  const DWORD index = TlsAlloc();
  *raw_index = index;
  if (index==TLS_OUT_OF_INDEXES) {
    mi_atomic_store_release(slot,MI_TLS_ERROR_SLOT);
    mi_atomic_store_release(extended,MI_TLS_ERROR_EXPANSION_SLOT);
    return false;
  }
  else if (index<MI_TLS_DIRECT_SLOTS) {
    mi_atomic_store_release(slot,index + MI_TLS_DIRECT_FIRST);
    mi_atomic_store_release(extended,0);
    return true;
  }
  #if !MI_WIN_DIRECT_TLS
  else if (index < MI_TLS_DIRECT_SLOTS + MI_TLS_EXPANSION_SLOTS - 1) { // check maximum number of expansion slots - 1 (as we use the last one as the default)
    mi_atomic_store_release(slot, MI_TLS_EXPANSION_SLOT);
    mi_atomic_store_release(extended,index - MI_TLS_DIRECT_SLOTS);
    return true;
  }
  #endif
  else {
    // to high an index for us
    _mi_error_message(EFAULT, "returned TLS index was too high (%u)\n", index);
    TlsFree(index);
    *raw_index = TLS_OUT_OF_INDEXES;
    mi_atomic_store_release(slot, MI_TLS_ERROR_SLOT);
    mi_atomic_store_release(extended,MI_TLS_ERROR_EXPANSION_SLOT);
    return false;
  }
}

static void mi_win_tls_slot_free(_Atomic(size_t)*slot, _Atomic(size_t)*extended, DWORD* raw_index) {
  if (*raw_index != TLS_OUT_OF_INDEXES) {
    mi_atomic_store_release(slot, MI_TLS_ERROR_SLOT);
    mi_atomic_store_release(extended, MI_TLS_ERROR_EXPANSION_SLOT);
    TlsFree(*raw_index);
    *raw_index = TLS_OUT_OF_INDEXES;
  }
}

void _mi_tls_slots_init(void) {
  mi_atomic_do_once {
    bool ok = mi_win_tls_slot_alloc(&_mi_theap_default_slot, &_mi_theap_default_expansion_slot, &mi_tls_raw_index_default);
    if (ok) {
      ok = mi_win_tls_slot_alloc(&_mi_theap_cached_slot, &_mi_theap_cached_expansion_slot, &mi_tls_raw_index_cached);
    }
    if (!ok) {
      _mi_error_message(EFAULT, "unable to allocate a fast TLS user slot.\n");
    }
  }
}

void _mi_tls_slots_done(void) {
  mi_win_tls_slot_free(&_mi_theap_default_slot, &_mi_theap_default_expansion_slot, &mi_tls_raw_index_default);
  mi_win_tls_slot_free(&_mi_theap_cached_slot, &_mi_theap_cached_expansion_slot, &mi_tls_raw_index_cached );
}

static void mi_win_tls_slot_set(size_t slot, size_t extended_slot, void* value) {
  mi_assert_internal((slot >= MI_TLS_DIRECT_FIRST && slot < MI_TLS_DIRECT_FIRST + MI_TLS_DIRECT_SLOTS) || slot == MI_TLS_EXPANSION_SLOT);
  if (slot < MI_TLS_DIRECT_FIRST + MI_TLS_DIRECT_SLOTS) {
    mi_prim_tls_slot_set(slot, value);
  }
  else {
    mi_assert_internal(extended_slot < MI_TLS_EXPANSION_SLOTS);
    TlsSetValue((DWORD)(extended_slot + MI_TLS_DIRECT_SLOTS), value);  // use TlsSetValue to initialize the TlsExpansion array if needed
  }
}

#elif MI_TLS_MODEL_PTHREADS

// only for pthreads for now
mi_decl_hidden _Atomic(pthread_key_t) _mi_theap_default_key = MI_ATOMIC_VAR_INIT(MI_PTHREAD_KEY_INVALID);
mi_decl_hidden _Atomic(pthread_key_t) _mi_theap_cached_key  = MI_ATOMIC_VAR_INIT(MI_PTHREAD_KEY_INVALID);

static void mi_theap_cached_key_destroy(void* theapv) {
  mi_theap_t* theap = (mi_theap_t*)mi_tls_auth(theapv, MI_TLS_TAG_THEAP_CACHED);
  if (theap!=NULL) {
    _mi_theap_decref(theap);
  }
}

// without the keys each allocation would re-initialize the thread
static void mi_tls_slots_init_once(void) {
  pthread_key_t key = MI_PTHREAD_KEY_INVALID;
  if (!_mi_pthread_key_create(&key,NULL,NULL)) { _mi_fatal_error("unable to allocate the thread local keys\n"); }
  mi_atomic_store_release(&_mi_theap_default_key,key);
  key = MI_PTHREAD_KEY_INVALID;
  if (!_mi_pthread_key_create(&key,&mi_theap_cached_key_destroy,NULL)) { _mi_fatal_error("unable to allocate the thread local keys\n"); }
  mi_atomic_store_release(&_mi_theap_cached_key,key);
}

void _mi_tls_slots_init(void) {
  mi_atomic_do_once { mi_tls_slots_init_once(); }
}

void _mi_tls_slots_done(void) {
  pthread_key_t key = mi_atomic_exchange_relaxed(&_mi_theap_default_key,MI_PTHREAD_KEY_INVALID);
  if (key!=MI_PTHREAD_KEY_INVALID) { pthread_key_delete(key); }
  key = mi_atomic_exchange_relaxed(&_mi_theap_cached_key,MI_PTHREAD_KEY_INVALID);
  if (key!=MI_PTHREAD_KEY_INVALID) { pthread_key_delete(key); }
}

#elif MI_TLS_MODEL_FIXED

void _mi_tls_slots_init(void) {
  mi_atomic_do_once {
    mi_theap_t* theap = _mi_theap_default();
    if (theap!=NULL) {
      _mi_error_message(EINVAL,"fixed TLS slot is already in use (slot %d = %p)", MI_TLS_MODEL_FIXED_DEFAULT, theap);
    }
    theap = _mi_theap_cached();
    if (theap!=NULL) {
      _mi_error_message(EINVAL,"fixed TLS slot is already in use (slot %d = %p)", MI_TLS_MODEL_FIXED_CACHED, theap);
    }
  }
}

void _mi_tls_slots_done(void) {
  // nothing
}


#else

void _mi_tls_slots_init(void) {
  // nothing
}

void _mi_tls_slots_done(void) {
  // nothing
}

#endif

void _mi_theap_cached_set(mi_theap_t* theap) {
  mi_theap_t* prev = _mi_theap_cached();
  if (prev==theap) return;
  // set
  _mi_tls_slots_init();
  void* const x = mi_tls_sign(theap, MI_TLS_TAG_THEAP_CACHED);
  #if MI_TLS_MODEL_LOCAL
    __mi_theap_cached = (mi_theap_t*)x;
  #elif MI_TLS_MODEL_FIXED
    mi_prim_tls_slot_set(MI_TLS_MODEL_FIXED_CACHED, x);
  #elif MI_TLS_MODEL_WIN32
    mi_win_tls_slot_set(mi_atomic_load_relaxed(&_mi_theap_cached_slot), mi_atomic_load_relaxed(&_mi_theap_cached_expansion_slot), x);
  #elif MI_TLS_MODEL_PTHREADS
    pthread_key_t key = mi_atomic_load_relaxed(&_mi_theap_cached_key);
    if (key!=MI_PTHREAD_KEY_INVALID) { pthread_setspecific(key, x); }
  #endif
  // update refcounts (so cached theap memory keeps available until no longer cached)
  _mi_theap_incref(theap);
  _mi_theap_decref(prev);
}

void _mi_theap_default_set(mi_theap_t* theap)  {
  mi_assert_internal(theap != NULL);
  mi_assert_internal(theap->tld != NULL);
  mi_assert_internal(mi_theap_matches_thread(theap));
  _mi_tls_slots_init();
  void* const x = mi_tls_sign(theap, MI_TLS_TAG_THEAP_DEFAULT);
  #if MI_TLS_MODEL_LOCAL
    __mi_theap_default = (mi_theap_t*)x;
  #elif MI_TLS_MODEL_FIXED
    mi_prim_tls_slot_set(MI_TLS_MODEL_FIXED_DEFAULT, x);
  #elif MI_TLS_MODEL_WIN32
    mi_win_tls_slot_set(mi_atomic_load_relaxed(&_mi_theap_default_slot), mi_atomic_load_relaxed(&_mi_theap_default_expansion_slot), x);
  #elif MI_TLS_MODEL_PTHREADS
    pthread_key_t key = mi_atomic_load_relaxed(&_mi_theap_default_key);
    if (key!=MI_PTHREAD_KEY_INVALID) { pthread_setspecific(key, x); }
  #endif

  // set theap main if needed
  if (mi_theap_is_initialized(theap)) {
    // ensure the default theap is passed to `_mi_thread_done` as on some platforms we cannot access TLS at thread termination (as it would allocate again)
    _mi_prim_thread_associate_default_theap((mi_theap_t*)mi_theap_thread_done_sign(theap));
  }
}

// Called on the slow path when the thread is not initialized: report an error if the
// default or cached theap entry was overwritten with an invalidly signed value (see `mi_theap_tls_auth`).
void _mi_theap_tls_check(void) {
  if (!MI_PAC) return;
  mi_tls_auth(mi_prim_theap_default_raw(), MI_TLS_TAG_THEAP_DEFAULT);
  mi_tls_auth(mi_prim_theap_cached_raw(), MI_TLS_TAG_THEAP_CACHED);
}
