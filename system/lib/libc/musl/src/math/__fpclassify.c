#include <math.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
// XXX Emscripten: for weak_alias.
#include "libc.h"
#endif

int __fpclassify(double x)
{
	union {double f; uint64_t i;} u = {x};
	int e = u.i>>52 & 0x7ff;
	if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
	if (e==0x7ff) return u.i<<12 ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}

#ifdef __EMSCRIPTEN__
// XXX Emscripten: Needed by tests/python/python.le32.bc
weak_alias(__fpclassify, __fpclassifyd);
#endif
