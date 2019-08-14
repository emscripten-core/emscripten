#include <math.h>

/* uses LONG_MAX > 2^24, see comments in lrint.c */

long lrintf(float x)
{
#ifdef __wasm__
	return __builtin_rintf(x);
#else
	return rintf(x);
#endif
}
