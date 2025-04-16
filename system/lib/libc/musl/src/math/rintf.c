#include <float.h>
#include <math.h>
#include <stdint.h>

#ifndef __wasm__
#if FLT_EVAL_METHOD==0
#define EPS FLT_EPSILON
#elif FLT_EVAL_METHOD==1
#define EPS DBL_EPSILON
#elif FLT_EVAL_METHOD==2
#define EPS LDBL_EPSILON
#endif
static const float_t toint = 1/EPS;
#endif

float rintf(float x)
{
// XXX EMSCRIPTEN: use the wasm instruction via clang builtin
// See https://github.com/emscripten-core/emscripten/issues/9236
#ifdef __wasm__
	return __builtin_rintf(x);
#else
	union {float f; uint32_t i;} u = {x};
	int e = u.i>>23 & 0xff;
	int s = u.i>>31;
	float_t y;

	if (e >= 0x7f+23)
		return x;
	if (s)
		y = x - toint + toint;
	else
		y = x + toint - toint;
	if (y == 0)
		return s ? -0.0f : 0.0f;
	return y;
#endif
}
