// This file uses SSE3 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <pmmintrin.h>
#define ENABLE_SSE2
#include "test_sse_full.h"

#ifndef _DEBUG
// The following tests break when optimizer is applied, so disable them for now. Baby steps.
// See https://github.com/kripken/emscripten/issues/3789
#define BREAKS_UNDER_OPTIMIZATION
#endif

float *interesting_floats = get_interesting_floats();
int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
uint32_t *interesting_ints = get_interesting_ints();
int numInterestingInts = sizeof(interesting_ints_)/sizeof(interesting_ints_[0]);
double *interesting_doubles = get_interesting_doubles();
int numInterestingDoubles = sizeof(interesting_doubles_)/sizeof(interesting_doubles_[0]);

int main()
{
	assert(numInterestingFloats % 4 == 0);
	assert(numInterestingInts % 4 == 0);
	assert(numInterestingDoubles % 4 == 0);	

	Ret_M128d_M128d(__m128d, _mm_addsub_pd);
	Ret_M128_M128(__m128, _mm_addsub_ps);
	Ret_M128d_M128d(__m128d, _mm_hadd_pd);
	Ret_M128_M128(__m128, _mm_hadd_ps);
	Ret_M128d_M128d(__m128d, _mm_hsub_pd);
	Ret_M128_M128(__m128, _mm_hsub_ps);
#ifndef BREAKS_UNDER_OPTIMIZATION
	Ret_IntPtr(__m128i, _mm_lddqu_si128, __m128i*, 4, 1);
#endif
	Ret_DoublePtr(__m128d, _mm_loaddup_pd, 1, 1);
	Ret_M128d(__m128d, _mm_movedup_pd);
	Ret_M128(__m128, _mm_movehdup_ps);
	Ret_M128(__m128, _mm_moveldup_ps);
}
