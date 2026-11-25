/* ----------------------------------------------------------------------------
Copyright (c) 2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_PRIM_PAC_H
#define MI_PRIM_PAC_H

#include "../types.h"
#include "../heap/process.h"   // mi_process_get_sealed_once

/* -----------------------------------------------------------------------------
Signed pointers (`MI_PAC`, see `arch.h`)

Some internal pointers (the page map entries, the arena of a memid, and the custom commit function)
are stored as a signed `mi_ptr_signed(tp)` (a `mi_ptr_signed_t`) that is bound to the address of the
slot it is stored in (the `salt`); use `mi_ptr_sign_at` and `mi_ptr_auth_at` (and the atomic variants)
to store and load such fields. Values that are copied (like the arena in a `mi_memid_t`) use a salt
derived from the other fields instead.
`NULL` is always signed as 0 (so zero initialized memory authenticates as `NULL`).

With memory tagging (`MI_MTE`) a signed pointer `p` must be `NULL` or aligned to
`MI_MEMTAG_UNIT_SIZE` (which holds for any tagged object start): the memory tag is moved
into the low 4 bits before signing (`q = mi_ptr_pac_untag(p)`) such that the top 16 bits
are available for the authentication code (and the tag is still authenticated).

- `MI_PAC=0` (off): the identity.
- `MI_PAC=1` (weak): `x = q ^ K ^ T(salt)` where `K` is a random process key in the sealed-once
  process data, and `T(salt) = salt >>> 18` rotates the (8-byte aligned) slot address such that its
  bits 3 to 17 end up in bits 49 to 63. An authenticated pointer must be canonical: the top 16 bits must be zero.
  This detects most forged values (if `K` is unknown), and always detects an entry that is copied to
  another slot in the same sub map. Since `K` always has bit 48 set (and `T(salt)` never has), the
  signature of `NULL` (0) never authenticates as canonical and is handled on the slow path.
- `MI_PAC=2` (required): `x = q | (pacga(q, salt) & MI_PAC_HI_MASK)` using the arm64 generic pointer
  authentication key (which cannot be read by the process). Pointer authentication must be supported by
  the CPU (checked at startup). `NULL` (0) either fails the authentication (and is handled on the slow
  path), or authenticates as `NULL`.

The free list links are always encoded with a MAC using per-page keys (see `alloc/page.h`), computed
by `mi_keyed_mac`. With `MI_PAC=1` the MAC also uses the process key `K` such that the free list cannot
be forged from just reading the page keys. With `MI_PAC=2` the MAC is `pacga(v, b ^ keys[0])` instead,
and the page offset is stored unmasked (as the page keys need not be secret).
Authentication failures report `EFAULT` and return `NULL` (see `_mi_error_ptr_auth_failed`).
----------------------------------------------------------------------------- */

// Sign a pointer `p` to be stored at address `salt`
static inline mi_ptr_signed_t mi_ptr_sign(const void* p, mi_addr_t salt);

// Authenticate a value `x` stored at address `salt`; `valid` is false if `x` is 0 or not validly signed
// (in which case `_mi_error_ptr_auth_failed` should be called)
static inline void* mi_ptr_auth_valid(mi_ptr_signed_t x, mi_addr_t salt, bool* valid);

// Authenticate a value `x` stored at address `salt`; returns NULL (and reports `EFAULT`) if `x` is not validly signed
static inline void* mi_ptr_auth(mi_ptr_signed_t x, mi_addr_t salt);

// Sign and authenticate an address without a memory tag (like a code pointer); this has no alignment requirement.
static inline mi_ptr_signed_t mi_addr_sign(mi_addr_t q, mi_addr_t salt);
static inline mi_addr_t mi_addr_auth_valid(mi_ptr_signed_t x, mi_addr_t salt, bool* valid);
static inline mi_addr_t mi_addr_auth(mi_ptr_signed_t x, mi_addr_t salt);

// Keyed MAC of a `value` bound to a `salt` for free list links (see `alloc/page.h:mi_ptr_mac`); uses the
// hardware key (`pacga`) or the process key. Only the high 32 bits are used (and with memory tagging also bits 24 to 31).
static inline mi_addr_t mi_keyed_mac(mi_addr_t value, mi_addr_t salt);

// Faster but weaker keyed MAC (using only the low 64 bits of the product), used for free list links with `MI_SECURE=1`.
static inline mi_addr_t mi_keyed_mac_fast(mi_addr_t value, mi_addr_t salt);


#if MI_PAC

// the top 16 bits must be zero in an authenticated pointer (after moving the memory tag to the low bits)
#define MI_PAC_ADDR_BITS    (48)
#define MI_PAC_HI_MASK      (~MI_ADDR(0) << MI_PAC_ADDR_BITS)

#if MI_MAX_VABITS > MI_PAC_ADDR_BITS
#error "signed pointers (MI_PAC) require at most 48-bit virtual addresses"
#endif

// Move the memory tag into the low bits (and back)
static inline mi_addr_t mi_ptr_pac_untag(const void* p) {
  #if MI_MTE
  const mi_addr_t u = (mi_addr_t)(uintptr_t)p;
  if mi_unlikely((u % MI_MEMTAG_UNIT_SIZE) != 0) { _mi_error_ptr_pac_unaligned(p); }
  return ((u & MI_PTR_ADDR_MASK) | ((u & MI_PTR_MEMTAG_MASK) >> MI_PTR_MEMTAG_SHIFT));
  #else
  return (mi_addr_t)(uintptr_t)p;
  #endif
}

static inline void* mi_ptr_pac_retag(mi_addr_t q) {
  #if MI_MTE
  const mi_addr_t t = (q % MI_MEMTAG_UNIT_SIZE);
  return (void*)(uintptr_t)((q ^ t) | (t << MI_PTR_MEMTAG_SHIFT));
  #else
  return (void*)(uintptr_t)q;
  #endif
}

#if MI_PAC==1
#define MI_PAC_KEY_ALWAYS   (MI_ADDR(1) << MI_PAC_ADDR_BITS)    // always set in the key so 0 never decodes as canonical

static inline mi_addr_t mi_pac_process_key(void) {
  mi_assert_internal(mi_process_get_sealed_once()->process_key != 0);  // initialized in `_mi_os_init`
  return mi_process_get_sealed_once()->process_key;
}

// the combined key for a slot: the process key xor-ed with the rotated slot address
static inline mi_addr_t mi_pac_process_slot_key(mi_addr_t slot_salt) {
  mi_assert_internal((slot_salt & 4) == 0);               // so bit `MI_PAC_ADDR_BITS` is never set
  return (mi_pac_process_key() ^ mi_rotr(slot_salt, 18));  // bit 2 to bit 48, and bits 3 to 17 to bits 49 to 63
}
#endif

#if MI_PAC>=2
// generic authentication code (in the high 32 bits); requires pointer authentication support (not a hint instruction)
static inline mi_addr_t mi_pac_ga(mi_addr_t x, mi_addr_t modifier) {
  register mi_addr_t x17 __asm__("x17") = x;
  register mi_addr_t x16 __asm__("x16") = modifier;
  __asm__ (".inst 0x9ad03231" : "+r"(x17) : "r"(x16));  // pacga x17, x17, x16
  return x17;
}
#endif

static inline mi_ptr_signed_t mi_addr_sign(mi_addr_t q, mi_addr_t salt) {
  if (q==0) return 0;
  #if MI_PAC >= 2
  return (mi_ptr_signed_t)(q | (mi_pac_ga(q, salt) & MI_PAC_HI_MASK));
  #else
  return (mi_ptr_signed_t)(q ^ mi_pac_process_slot_key(salt));
  #endif
}

static inline mi_addr_t mi_addr_auth_valid(mi_ptr_signed_t x, mi_addr_t salt, bool* valid) {
  #if MI_PAC >= 2
  // no special case for 0: it either fails the authentication, or authenticates as NULL
  const mi_addr_t q = (mi_addr_t)x & ~MI_PAC_HI_MASK;
  *valid = (((mi_pac_ga(q, salt) ^ (mi_addr_t)x) & MI_PAC_HI_MASK) == 0);
  #else
  const mi_addr_t q = (mi_addr_t)x ^ mi_pac_process_slot_key(salt);
  *valid = ((q & MI_PAC_HI_MASK) == 0);
  #endif
  return q;
}

static inline mi_ptr_signed_t mi_ptr_sign(const void* p, mi_addr_t salt) {
  return mi_addr_sign(mi_ptr_pac_untag(p), salt);
}

static inline void* mi_ptr_auth_valid(mi_ptr_signed_t x, mi_addr_t salt, bool* valid) {
  return mi_ptr_pac_retag(mi_addr_auth_valid(x, salt, valid));
}

static inline mi_addr_t mi_keyed_mac(mi_addr_t value, mi_addr_t salt) {
  #if MI_PAC >= 2
  const mi_addr_t m = mi_pac_ga(value, salt);
  return (m | (m >> 32));
  #else
  return mi_mul_xor64(value, salt ^ mi_pac_process_key());
  #endif
}

static inline mi_addr_t mi_keyed_mac_fast(mi_addr_t value, mi_addr_t salt) {
  #if MI_PAC >= 2
  return mi_keyed_mac(value, salt);
  #else
  return (value * (salt ^ mi_pac_process_key()));
  #endif
}

#else // !MI_PAC

static inline mi_ptr_signed_t mi_addr_sign(mi_addr_t q, mi_addr_t salt) {
  MI_UNUSED(salt);
  return (mi_ptr_signed_t)q;
}

static inline mi_addr_t mi_addr_auth_valid(mi_ptr_signed_t x, mi_addr_t salt, bool* valid) {
  MI_UNUSED(salt);
  *valid = true;
  return (mi_addr_t)x;
}

static inline mi_ptr_signed_t mi_ptr_sign(const void* p, mi_addr_t salt) {
  MI_UNUSED(salt);
  return (mi_ptr_signed_t)p;
}

static inline void* mi_ptr_auth_valid(mi_ptr_signed_t x, mi_addr_t salt, bool* valid) {
  MI_UNUSED(salt);
  *valid = true;
  return (void*)x;
}

static inline mi_addr_t mi_keyed_mac(mi_addr_t value, mi_addr_t salt) {
  return (mi_addr_t)mi_mul_xor64(value, salt);
}

static inline mi_addr_t mi_keyed_mac_fast(mi_addr_t value, mi_addr_t salt) {
  return (value * salt);
}

#endif // MI_PAC/!MI_PAC

static inline void* mi_ptr_auth(mi_ptr_signed_t x, mi_addr_t salt) {
  bool valid;
  void* const p = mi_ptr_auth_valid(x, salt, &valid);
  if mi_unlikely(!valid) return _mi_error_ptr_auth_failed(x, salt);
  return p;
}

static inline mi_addr_t mi_addr_auth(mi_ptr_signed_t x, mi_addr_t salt) {
  bool valid;
  const mi_addr_t q = mi_addr_auth_valid(x, salt, &valid);
  if mi_unlikely(!valid) return mi_addr(_mi_error_ptr_auth_failed(x, salt));
  return q;
}

// Derive a salt from a value (salts must be 8-byte aligned)
static inline mi_addr_t mi_salt_of(size_t v) {
  return ((mi_addr_t)v << 3);
}


/* -----------------------------------------------------------------------------
Typed signed pointer fields salted with their slot address. A field is declared as
`mi_ptr_signed(tp) field` (or `_Atomic(mi_ptr_signed(tp))`), and stored and loaded using
`mi_ptr_sign_at(&field,p)` and `mi_ptr_auth_at(tp,&field)` (or the atomic variants).
Code pointers use `mi_fun_sign_at` and `mi_fun_auth_at` (as these have no memory tag or alignment).
Note: the `slot` argument is evaluated twice.
----------------------------------------------------------------------------- */

#define mi_ptr_sign_at(slot,p)                (*(slot) = mi_ptr_sign(p, mi_addr(slot)))
#define mi_ptr_auth_at(tp,slot)               ((tp*)mi_ptr_auth(*(slot), mi_addr(slot)))

#define mi_fun_sign_at(slot,f)                (*(slot) = mi_addr_sign((mi_addr_t)(uintptr_t)(f), mi_addr(slot)))
#define mi_fun_auth_at(tp,slot)               ((tp*)(uintptr_t)mi_addr_auth(*(slot), mi_addr(slot)))

#define mi_atomic_ptr_sign_relaxed(slot,p)    mi_atomic_store_relaxed(slot, mi_ptr_sign(p, mi_addr(slot)))
#define mi_atomic_ptr_sign_release(slot,p)    mi_atomic_store_release(slot, mi_ptr_sign(p, mi_addr(slot)))
#define mi_atomic_ptr_auth_relaxed(tp,slot)   ((tp*)mi_ptr_auth(mi_atomic_load_relaxed(slot), mi_addr(slot)))
#define mi_atomic_ptr_auth_acquire(tp,slot)   ((tp*)mi_ptr_auth(mi_atomic_load_acquire(slot), mi_addr(slot)))

#endif // MI_PRIM_PAC_H
