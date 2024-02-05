#include <math.h>
#include <stdint.h>

float fabsf(float x)
{
// XXX EMSCRIPTEN: use the wasm instruction via clang builtin
// See https://github.com/emscripten-core/emscripten/issues/9236
#ifdef __wasm__
	return __builtin_fabsf(x);
#else
	union {float f; uint32_t i;} u = {x};
	u.i &= 0x7fffffff;
	return u.f;
#endif
}
