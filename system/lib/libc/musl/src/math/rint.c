#include <float.h>
#include <math.h>
#include <stdint.h>

#ifndef __wasm__
#if FLT_EVAL_METHOD==0 || FLT_EVAL_METHOD==1
#define EPS DBL_EPSILON
#elif FLT_EVAL_METHOD==2
#define EPS LDBL_EPSILON
#endif
static const double_t toint = 1/EPS;
#endif

double rint(double x)
{
// XXX EMSCRIPTEN: use the wasm instruction via clang builtin
// See https://github.com/emscripten-core/emscripten/issues/9236
#ifdef __wasm__
	return __builtin_rint(x);
#else
	union {double f; uint64_t i;} u = {x};
	int e = u.i>>52 & 0x7ff;
	int s = u.i>>63;
	double_t y;

	if (e >= 0x3ff+52)
		return x;
	if (s)
		y = x - toint + toint;
	else
		y = x + toint - toint;
	if (y == 0)
		return s ? -0.0 : 0;
	return y;
#endif
}
