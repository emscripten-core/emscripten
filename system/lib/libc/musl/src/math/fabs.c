#include <math.h>
#include <stdint.h>

double fabs(double x)
{
// XXX EMSCRIPTEN: use the wasm instruction via clang builtin
// See https://github.com/emscripten-core/emscripten/issues/9236
#ifdef __wasm__
	return __builtin_fabs(x);
#else
	union {double f; uint64_t i;} u = {x};
	u.i &= -1ULL/2;
	return u.f;
#endif
}
