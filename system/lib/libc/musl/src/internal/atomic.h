#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <stdint.h>

#include "atomic_arch.h"

// XXX EMSCRIPTEN: Default/fallback implementations removed for clarity since
// emscripten defines everything in atomic_arch.h.

static inline int a_ctz_l(unsigned long x)
{
	return (sizeof(long) < 8) ? a_ctz_32(x) : a_ctz_64(x);
}

#endif
