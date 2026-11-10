/**
 * Emscripten/C11-specific version of atomic_arch.h.
 *
 * Here we implement all of musl's atomics APIs (a_*) using Clang's
 * C11 atomic builtins (__c11_atomic_*) that LLVM can then lower to
 * Wasm atomic instructions.
 *
 * Note: We also change all atomic pointer types from `volatile xx*`
 * to `_Atomic xx*` which have well-defined semantics in C11/LLVM.
 */

#ifndef _INTERNAL_ATOMIC_H
#define _INTERNAL_ATOMIC_H

#include <stdint.h>

#define a_clz_32 __builtin_clz
#define a_ctz_32 __builtin_ctz
#define a_clz_64 __builtin_clzll
#define a_ctz_64 __builtin_ctzll

#define a_and_64 a_and_64
static inline void a_and_64(_Atomic uint64_t *p, uint64_t v)
{
	__c11_atomic_fetch_and(p, v, __ATOMIC_SEQ_CST);
}

#define a_or_64 a_or_64
static inline void a_or_64(_Atomic uint64_t *p, uint64_t v)
{
	__c11_atomic_fetch_or(p, v, __ATOMIC_SEQ_CST);
}

#define a_or_l a_or_l
static inline void a_or_l(_Atomic long *p, long v)
{
	__c11_atomic_fetch_or(p, v, __ATOMIC_SEQ_CST);
}

// `p` is a non-atomic pointer to an atomic `void *` (equivalent to `void * _Atomic *p`).
#define a_cas_p a_cas_p
static inline void *a_cas_p(_Atomic(void *) *p, void *t, void *s)
{
	__c11_atomic_compare_exchange_strong(p, &t, s, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return t;
}

#define a_cas a_cas
static inline int a_cas(_Atomic int *p, int t, int s)
{
	__c11_atomic_compare_exchange_strong(p, &t, s, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return t;
}

#define a_fetch_or a_fetch_or
static inline int a_fetch_or(_Atomic int *p, int v)
{
	return __c11_atomic_fetch_or(p, v, __ATOMIC_SEQ_CST);
}

#define a_fetch_and a_fetch_and
static inline int a_fetch_and(_Atomic int *p, int v)
{
	return __c11_atomic_fetch_and(p, v, __ATOMIC_SEQ_CST);
}

#define a_or a_or
static inline void a_or(_Atomic int *p, int v)
{
	__c11_atomic_fetch_or(p, v, __ATOMIC_SEQ_CST);
}

#define a_and a_and
static inline void a_and(_Atomic int *p, int v)
{
	__c11_atomic_fetch_and(p, v, __ATOMIC_SEQ_CST);
}

#define a_swap a_swap
static inline int a_swap(_Atomic int *x, int v)
{
	return __c11_atomic_exchange(x, v, __ATOMIC_SEQ_CST);
}

#define a_fetch_add a_fetch_add
static inline int a_fetch_add(_Atomic int *x, int v)
{
	return __c11_atomic_fetch_add(x, v, __ATOMIC_SEQ_CST);
}

#define a_inc a_inc
static inline void a_inc(_Atomic int *x)
{
	__c11_atomic_fetch_add(x, 1, __ATOMIC_SEQ_CST);
}

#define a_dec a_dec
static inline void a_dec(_Atomic int *x)
{
	__c11_atomic_fetch_sub(x, 1, __ATOMIC_SEQ_CST);
}

#define a_store a_store
static inline void a_store(_Atomic int *p, int x)
{
	__c11_atomic_store(p, x, __ATOMIC_SEQ_CST);
}

#define a_barrier a_barrier
static inline void a_barrier()
{
	__c11_atomic_thread_fence(__ATOMIC_SEQ_CST);
}

#define a_spin a_spin
static inline void a_spin()
{
}

#define a_crash a_crash
static inline void a_crash()
{
	__builtin_trap();
}

#endif
