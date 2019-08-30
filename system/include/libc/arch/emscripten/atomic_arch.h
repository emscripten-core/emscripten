#ifndef _INTERNAL_ATOMIC_H
#define _INTERNAL_ATOMIC_H

#include <stdint.h>
#include <emscripten.h>
#include <emscripten/threading.h>

#define a_ctz_l a_ctz_l
static inline int a_ctz_l(unsigned long x)
{
	if (x == 0)
		return 32;
	int nTrailingZeros = 0;
	while(!(x&1))
	{
		++nTrailingZeros;
		x >>= 1;
	}
	return nTrailingZeros;
}

#define a_ctz_64 a_ctz_64
static inline int a_ctz_64(uint64_t x)
{
	uint32_t lo = (uint32_t)x;
	if (lo == 0)
		return a_ctz_l((unsigned long)(x >> 32)) + 32;
	else
		return a_ctz_l((unsigned long)lo);
}

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

#ifdef __EMSCRIPTEN_PTHREADS__
#ifdef __asmjs__
#define a_store_l a_store_l
static inline void a_store_l(volatile void *p, long x)
{
	emscripten_atomic_store_u32((void*)p, x);
}

#define a_or_l a_or_l
static inline void a_or_l(volatile void *p, long v)
{
	emscripten_atomic_or_u32((void*)p, v);
}

#define a_cas_p a_cas_p
static inline void *a_cas_p(volatile void *p, void *t, void *s)
{
	return (void*)emscripten_atomic_cas_u32(p, (uint32_t)t, (uint32_t)s);
}

#define a_cas_l a_cas_l
static inline long a_cas_l(volatile void *p, long t, long s)
{
	return emscripten_atomic_cas_u32(p, t, s);
}

#define a_cas a_cas
static inline int a_cas(volatile int *p, int t, int s)
{
	return emscripten_atomic_cas_u32(p, t, s);
}

#define a_or a_or
static inline void a_or(volatile void *p, int v)
{
	emscripten_atomic_or_u32((void*)p, v);
}

#define a_and a_and
static inline void a_and(volatile void *p, int v)
{
	emscripten_atomic_and_u32((void*)p, v);
}

#define a_swap a_swap
static inline int a_swap(volatile int *x, int v)
{
	int old;
	do {
		old = emscripten_atomic_load_u32(x);
	} while(emscripten_atomic_cas_u32(x, old, v) != old);
	return old;
}

#define a_fetch_add a_fetch_add
static inline int a_fetch_add(volatile int *x, int v)
{
	return emscripten_atomic_add_u32(x, v);
}

#define a_inc a_inc
static inline void a_inc(volatile int *x)
{
	emscripten_atomic_add_u32((void*)x, 1);
}

#define a_dec a_dec
static inline void a_dec(volatile int *x)
{
	emscripten_atomic_sub_u32((void*)x, 1);
}

#define a_store a_store
static inline void a_store(volatile int *p, int x)
{
	emscripten_atomic_store_u32((void*)p, x);
}
#else // __asmjs__
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
	void* expected = t;
	__c11_atomic_compare_exchange_strong((_Atomic uintptr_t*)p, &expected, s, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return expected;
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
#endif // __asmjs__
#else // __EMSCRIPTEN_PTHREADS__
#define a_store_l a_store_l
static inline void a_store_l(volatile void *p, long x)
{
	*(long*)p = x;
}

#define a_or_l a_or_l
static inline void a_or_l(volatile void *p, long v)
{
	*(long*)p |= v;
}

#define a_cas_p a_cas_p
static inline void *a_cas_p(volatile void *p, void *t, void *s)
{
	if (*(long*)p == t)
		*(long*)p = s;
	return t;
}

#define a_cas_l a_cas_l
static inline long a_cas_l(volatile void *p, long t, long s)
{
	if (*(long*)p == t)
		*(long*)p = s;
	return t;
}

#define a_cas a_cas
static inline int a_cas(volatile int *p, int t, int s)
{
	if (*p == t)
		*p = s;
	return t;
}

#define a_or a_or
static inline void a_or(volatile void *p, int v)
{
	*(int*)p |= v;
}

#define a_and a_and
static inline void a_and(volatile void *p, int v)
{
	*(int*)p &= v;
}

#define a_inc a_inc
static inline void a_inc(volatile int *x)
{
	++*x;
}

#define a_dec a_dec
static inline void a_dec(volatile int *x)
{
	--*x;
}

#define a_store a_store
static inline void a_store(volatile int *p, int x)
{
	*p = x;
}

#define a_swap a_swap
static inline int a_swap(volatile int *x, int v)
{
	int old;
	do old = *x;
	while (!__sync_bool_compare_and_swap(x, old, v));
	return old;
}

#define a_fetch_add a_fetch_add
static inline int a_fetch_add(volatile int *x, int v)
{
  return __sync_fetch_and_add(x, v);
}
#endif

#define a_spin a_spin
static inline void a_spin()
{
}

#define a_crash a_crash
static inline void a_crash()
{
  EM_ASM( abort() );
}

#endif
