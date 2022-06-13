/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses AVX by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <immintrin.h>
#include "test_sse.h"

bool testNaNBits = true;

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

	Ret_FloatPtr(__m128, _mm_broadcast_ss, 1, 1);
	Ret_M128d_M128d_Tint_5bits(__m128d, _mm_cmp_pd);
	Ret_M128_M128_Tint_5bits(__m128, _mm_cmp_ps);
	Ret_M128d_M128d_Tint_5bits(__m128d, _mm_cmp_sd);
	Ret_M128_M128_Tint_5bits(__m128, _mm_cmp_ss);
	Ret_DoublePtr_M128i(__m128d, _mm_maskload_pd, 2, 2);
	Ret_FloatPtr_M128i(__m128, _mm_maskload_ps, 4, 4);
	void_OutDoublePtr_M128i_M128d(_mm_maskstore_pd, double*, 16, 8);
	void_OutFloatPtr_M128i_M128(_mm_maskstore_ps, float*, 16, 4);
	Ret_M128d_Tint(__m128d, _mm_permute_pd);
	Ret_M128_Tint(__m128, _mm_permute_ps);
	Ret_M128d_M128d(__m128d, _mm_permutevar_pd);
	Ret_M128_M128(__m128, _mm_permutevar_ps);
	Ret_M128d_M128d(int, _mm_testc_pd);
	Ret_M128_M128(int, _mm_testc_ps);
	Ret_M128d_M128d(int, _mm_testnzc_pd);
	Ret_M128_M128(int, _mm_testnzc_ps);
	Ret_M128d_M128d(int, _mm_testz_pd);
	Ret_M128_M128(int, _mm_testz_ps);
}
