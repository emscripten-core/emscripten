// This file uses SSE1 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <xmmintrin.h>
#include "test_sse_full.h"

int main()
{
	float *interesting_floats = get_interesting_floats();
	int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
	assert(numInterestingFloats % 4 == 0);

	uint32_t *interesting_ints = get_interesting_ints();
	int numInterestingInts = sizeof(interesting_ints_)/sizeof(interesting_ints_[0]);
	assert(numInterestingInts % 4 == 0);

	// SSE1 Arithmetic instructions:
	Ret_M128_M128(__m128, _mm_add_ps);
	Ret_M128_M128(__m128, _mm_add_ss);
	Ret_M128_M128(__m128, _mm_div_ps);
	Ret_M128_M128(__m128, _mm_div_ss);
	Ret_M128_M128(__m128, _mm_mul_ps);
	Ret_M128_M128(__m128, _mm_mul_ss);
	Ret_M128_M128(__m128, _mm_sub_ps);
	Ret_M128_M128(__m128, _mm_sub_ss);

	// SSE1 Elementary Math functions:
#if 0 // TODO: Precision differs in SIMD.js and native. Test differently. See https://github.com/kripken/emscripten/issues/3049
	Ret_M128(__m128, _mm_rcp_ps);
	Ret_M128(__m128, _mm_rcp_ss);
	Ret_M128(__m128, _mm_rsqrt_ps);
	Ret_M128(__m128, _mm_rsqrt_ss);
	Ret_M128(__m128, _mm_sqrt_ps);
	Ret_M128(__m128, _mm_sqrt_ss);
#endif

	// SSE1 Logical instructions:
	Ret_M128_M128(__m128, _mm_and_ps);
	Ret_M128_M128(__m128, _mm_andnot_ps);
	Ret_M128_M128(__m128, _mm_or_ps);
	Ret_M128_M128(__m128, _mm_xor_ps);

	// SSE1 Compare instructions:
	Ret_M128_M128(__m128, _mm_cmpeq_ps);
	Ret_M128_M128(__m128, _mm_cmpeq_ss);
	Ret_M128_M128(__m128, _mm_cmpge_ps);
	Ret_M128_M128(__m128, _mm_cmpge_ss);
	Ret_M128_M128(__m128, _mm_cmpgt_ps);
	Ret_M128_M128(__m128, _mm_cmpgt_ss);
	Ret_M128_M128(__m128, _mm_cmple_ps);
	Ret_M128_M128(__m128, _mm_cmple_ss);
	Ret_M128_M128(__m128, _mm_cmplt_ps);
	Ret_M128_M128(__m128, _mm_cmplt_ss);
	Ret_M128_M128(__m128, _mm_cmpneq_ps);
	Ret_M128_M128(__m128, _mm_cmpneq_ss);
	Ret_M128_M128(__m128, _mm_cmpnge_ps);
	Ret_M128_M128(__m128, _mm_cmpnge_ss);
	Ret_M128_M128(__m128, _mm_cmpngt_ps);
	Ret_M128_M128(__m128, _mm_cmpngt_ss);
	Ret_M128_M128(__m128, _mm_cmpnle_ps);
	Ret_M128_M128(__m128, _mm_cmpnle_ss);
	Ret_M128_M128(__m128, _mm_cmpnlt_ps);
	Ret_M128_M128(__m128, _mm_cmpnlt_ss);
	Ret_M128_M128(__m128, _mm_cmpord_ps);
	Ret_M128_M128(__m128, _mm_cmpord_ss);
	Ret_M128_M128(__m128, _mm_cmpunord_ps);
	Ret_M128_M128(__m128, _mm_cmpunord_ss);

	Ret_M128_M128(int, _mm_comieq_ss);
	Ret_M128_M128(int, _mm_comige_ss);
	Ret_M128_M128(int, _mm_comigt_ss);
	Ret_M128_M128(int, _mm_comile_ss);
	Ret_M128_M128(int, _mm_comilt_ss);
	Ret_M128_M128(int, _mm_comineq_ss);
	Ret_M128_M128(int, _mm_ucomieq_ss);
	Ret_M128_M128(int, _mm_ucomige_ss);
	Ret_M128_M128(int, _mm_ucomigt_ss);
	Ret_M128_M128(int, _mm_ucomile_ss);
	Ret_M128_M128(int, _mm_ucomilt_ss);
	Ret_M128_M128(int, _mm_ucomineq_ss);

	// SSE1 Convert instructions:
	Ret_M128_int(__m128, _mm_cvt_si2ss);
	Ret_M128(int, _mm_cvt_ss2si);
	Ret_M128_int(__m128, _mm_cvtsi32_ss);
	Ret_M128(float, _mm_cvtss_f32);
	Ret_M128(int, _mm_cvtss_si32);
	Ret_M128(int64_t, _mm_cvtss_si64);
	Ret_M128(int, _mm_cvtt_ss2si);
	Ret_M128(int, _mm_cvttss_si32);
	Ret_M128(int64_t, _mm_cvttss_si64);

	// SSE1 Load functions:
	Ret_FloatPtr(__m128, _mm_load_ps, 4, 4);
	Ret_FloatPtr(__m128, _mm_load_ps1, 1, 1);
	Ret_FloatPtr(__m128, _mm_load_ss, 1, 1);
	Ret_FloatPtr(__m128, _mm_load1_ps, 1, 1);
	Ret_M128_FloatPtr(__m128, _mm_loadh_pi, __m64*, 2, 1);
	Ret_M128_FloatPtr(__m128, _mm_loadl_pi, __m64*, 2, 1);
	Ret_FloatPtr(__m128, _mm_loadr_ps, 4, 4);
	Ret_FloatPtr(__m128, _mm_loadu_ps, 4, 1);

	// SSE1 Miscellaneous functions:
	Ret_M128(int, _mm_movemask_ps);

	// SSE1 Move functions:
	Ret_M128_M128(__m128, _mm_move_ss);
	Ret_M128_M128(__m128, _mm_movehl_ps);
	Ret_M128_M128(__m128, _mm_movelh_ps);

/*
	// SSE1 Set functions:
	_mm_set_ps
	_mm_set_ps1
	_mm_set_ss
	_mm_set1_ps
	_mm_setr_ps
	_mm_setzero_ps
*/

	// SSE1 Special Math instructions:
	Ret_M128_M128(__m128, _mm_max_ps);
	Ret_M128_M128(__m128, _mm_max_ss);
	Ret_M128_M128(__m128, _mm_min_ps);
	Ret_M128_M128(__m128, _mm_min_ss);

	// SSE1 Store instructions:
	void_OutFloatPtr_M128(_mm_store_ps, float*, 16, 16);
	void_OutFloatPtr_M128(_mm_store_ps1, float*, 16, 16);
	void_OutFloatPtr_M128(_mm_store_ss, float*, 4, 1);
	void_OutFloatPtr_M128(_mm_store1_ps, float*, 16, 16);
	void_OutFloatPtr_M128(_mm_storeh_pi, __m64*, 8, 1);
	void_OutFloatPtr_M128(_mm_storel_pi, __m64*, 8, 1);
	void_OutFloatPtr_M128(_mm_storer_ps, float*, 16, 16);
	void_OutFloatPtr_M128(_mm_storeu_ps, float*, 16, 1);
	void_OutFloatPtr_M128(_mm_stream_ps, float*, 16, 16);

	// SSE1 Swizzle instructions:
	Ret_M128_M128_Tint(__m128, _mm_shuffle_ps);
	// _MM_TRANSPOSE4_PS
	Ret_M128_M128(__m128, _mm_unpackhi_ps);
	Ret_M128_M128(__m128, _mm_unpacklo_ps);
}
