#ifndef _INTERNAL_ATOMIC_H
#define _INTERNAL_ATOMIC_H

#include <stdint.h>

#define a_clz_l __builtin_clz
#define a_ctz_l __builtin_ctz
#define a_clz_64 __builtin_clzll
#define a_ctz_64 __builtin_ctzll

#define a_and_64 a_and_64
static inline void a_and_64(volatile uint64_t *p, uint64_t v)
{
	*p &= v;
}

#define a_or_64 a_or_64
static inline void a_or_64(volatile uint64_t *p, uint64_t v)
{
	*p |= v;
}

#define a_store_l a_store_l
static inline void a_store_l(volatile void *p, long x)
{
	__c11_atomic_store((_Atomic long*)p, x, __ATOMIC_SEQ_CST);
}

#define a_or_l a_or_l
static inline void a_or_l(volatile void *p, long v)
{
	__c11_atomic_fetch_or((_Atomic long*)p, v, __ATOMIC_SEQ_CST);
}
#define a_cas_p a_cas_p
static inline void *a_cas_p(volatile void *p, void *t, void *s)
{
	uintptr_t expected = (uintptr_t)t;
	__c11_atomic_compare_exchange_strong((_Atomic uintptr_t*)p, &expected, (uintptr_t)s, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return (void*)expected;
}

#define a_cas_l a_cas_l
static inline long a_cas_l(volatile void *p, long t, long s)
{
	long expected = t;
	__c11_atomic_compare_exchange_strong((_Atomic long*)p, &expected, s, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return expected;
}

#define a_cas a_cas
static inline int a_cas(volatile int *p, int t, int s)
{
	int expected = t;
	__c11_atomic_compare_exchange_strong((_Atomic int*)p, &expected, s, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return expected;
}

#define a_or a_or
static inline void a_or(volatile void *p, int v)
{
	__c11_atomic_fetch_or((_Atomic int*)p, v, __ATOMIC_SEQ_CST);
}

#define a_and a_and
static inline void a_and(volatile void *p, int v)
{
	__c11_atomic_fetch_and((_Atomic int*)p, v, __ATOMIC_SEQ_CST);
}

#define a_swap a_swap
static inline int a_swap(volatile int *x, int v)
{
	return __c11_atomic_exchange((_Atomic int*)x, v, __ATOMIC_SEQ_CST);
}

#define a_fetch_add a_fetch_add
static inline int a_fetch_add(volatile int *x, int v)
{
	return __c11_atomic_fetch_add((_Atomic int*)x, v, __ATOMIC_SEQ_CST);
}

#define a_inc a_inc
static inline void a_inc(volatile int *x)
{
	__c11_atomic_fetch_add((_Atomic int*)x, 1, __ATOMIC_SEQ_CST);
}

#define a_dec a_dec
static inline void a_dec(volatile int *x)
{
	__c11_atomic_fetch_sub((_Atomic int*)x, 1, __ATOMIC_SEQ_CST);
}

#define a_store a_store
static inline void a_store(volatile int *p, int x)
{
	__c11_atomic_store((_Atomic int*)p, x, __ATOMIC_SEQ_CST);
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
