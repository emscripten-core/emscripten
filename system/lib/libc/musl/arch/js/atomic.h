#ifndef _INTERNAL_ATOMIC_H
#define _INTERNAL_ATOMIC_H

#include <stdint.h>

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

static inline int a_ctz_64(uint64_t x)
{
	uint32_t lo = (uint32_t)x;
	if (lo == 0)
		return a_ctz_l((unsigned long)(x >> 32)) + 32;
	else
		return a_ctz_l((unsigned long)lo);
}

static inline void a_and_64(volatile uint64_t *p, uint64_t v)
{
	*p &= v;
}

static inline void a_or_64(volatile uint64_t *p, uint64_t v)
{
	*p |= v;
}

static inline void a_store_l(volatile void *p, long x)
{
	*(long*)p = x;
}

static inline void a_or_l(volatile void *p, long v)
{
	*(long*)p |= v;
}

static inline void *a_cas_p(volatile void *p, void *t, void *s)
{
	if (*(long*)p == t)
		*(long*)p = s;
	return t;
}

static inline long a_cas_l(volatile void *p, long t, long s)
{
	if (*(long*)p == t)
		*(long*)p = s;
	return t;
}

static inline int a_cas(volatile int *p, int t, int s)
{
	if (*p == t)
		*p = s;
	return t;
}

static inline void a_or(volatile void *p, int v)
{
	*(int*)p |= v;
}

static inline void a_and(volatile void *p, int v)
{
	*(int*)p &= v;
}

static inline void a_inc(volatile int *x)
{
	++*x;
}

static inline void a_dec(volatile int *x)
{
	--*x;
}

static inline void a_store(volatile int *p, int x)
{
	*p = x;
}

static inline void a_spin()
{
}

static inline void a_crash()
{
}


#endif
