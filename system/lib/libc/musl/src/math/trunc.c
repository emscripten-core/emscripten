#include "libm.h"

double trunc(double x)
{
// XXX EMSCRIPTEN: use the wasm instruction via clang builtin
// See https://github.com/emscripten-core/emscripten/issues/9236
#ifdef __wasm__
	return __builtin_trunc(x);
#else
	union {double f; uint64_t i;} u = {x};
	int e = (int)(u.i >> 52 & 0x7ff) - 0x3ff + 12;
	uint64_t m;

	if (e >= 52 + 12)
		return x;
	if (e < 12)
		e = 1;
	m = -1ULL >> e;
	if ((u.i & m) == 0)
		return x;
	FORCE_EVAL(x + 0x1p120f);
	u.i &= ~m;
	return u.f;
#endif
}
