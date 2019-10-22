#include <math.h>

double fmin(double x, double y)
{
	if (isnan(x))
		return y;
	if (isnan(y))
		return x;
#ifdef __EMSCRIPTEN__
	return __builtin_wasm_min_f64(x, y);
#else
	/* handle signed zeros, see C99 Annex F.9.9.2 */
	if (signbit(x) != signbit(y))
		return signbit(x) ? x : y;
	return x < y ? x : y;
#endif
}
