// This file uses SSE2 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <emmintrin.h>
#define ENABLE_SSE2
#include "test_sse_full.h"

// We don't have an Int64x2 type, but we do emulate in scalar Int64x2 code. However, 
// the PNaCl ExpandI64.cpp path fails to expand our emulated code when LLVM wants to pack or
// unpack the 64bit elements to/from vectors. Therefore skip that path for now in release builds.
// (debug builds work ok, since they avoid optimized smartness)
// See https://github.com/kripken/emscripten/issues/3788
#ifndef _DEBUG
#define NO_INT64X2
#endif

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

void test_arithmetic()
{
	// SSE2 Arithmetic instructions:
	M128i_M128i_M128i(_mm_add_epi16);
	M128i_M128i_M128i(_mm_add_epi32);
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_add_epi64);
#endif
	M128i_M128i_M128i(_mm_add_epi8);
	Ret_M128d_M128d(__m128d, _mm_add_pd);
	Ret_M128d_M128d(__m128d, _mm_add_sd);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_adds_epi16);
#endif
	M128i_M128i_M128i(_mm_adds_epi8);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_adds_epu16);
#endif
	M128i_M128i_M128i(_mm_adds_epu8);
	Ret_M128d_M128d(__m128d, _mm_div_pd);
	Ret_M128d_M128d(__m128d, _mm_div_sd);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_madd_epi16);
#endif
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_mul_epu32);
#endif
	Ret_M128d_M128d(__m128d, _mm_mul_pd);
	Ret_M128d_M128d(__m128d, _mm_mul_sd);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_mulhi_epi16);
	M128i_M128i_M128i(_mm_mulhi_epu16);
#endif
	M128i_M128i_M128i(_mm_mullo_epi16);
	M128i_M128i_M128i(_mm_sad_epu8);
	M128i_M128i_M128i(_mm_sub_epi16);
	M128i_M128i_M128i(_mm_sub_epi32);
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_sub_epi64);
#endif
	M128i_M128i_M128i(_mm_sub_epi8);
	Ret_M128d_M128d(__m128d, _mm_sub_pd);
	Ret_M128d_M128d(__m128d, _mm_sub_sd);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_subs_epi16);
#endif
	M128i_M128i_M128i(_mm_subs_epi8);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_subs_epu16);
#endif
	M128i_M128i_M128i(_mm_subs_epu8);
}

void test_cast()
{
	// SSE2 Cast functions:
	Ret_M128d(__m128, _mm_castpd_ps);
	Ret_M128d(__m128i, _mm_castpd_si128);
	Ret_M128(__m128d, _mm_castps_pd);
	Ret_M128(__m128i, _mm_castps_si128);
	Ret_M128i(__m128d, _mm_castsi128_pd);
	Ret_M128i(__m128, _mm_castsi128_ps);
}

void test_compare()
{
	// SSE2 Compare instructions:
	M128i_M128i_M128i(_mm_cmpeq_epi16);
	M128i_M128i_M128i(_mm_cmpeq_epi32);
	M128i_M128i_M128i(_mm_cmpeq_epi8);
	Ret_M128d_M128d(__m128d, _mm_cmpeq_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpeq_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpge_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpge_sd);
	M128i_M128i_M128i(_mm_cmpgt_epi16);
	M128i_M128i_M128i(_mm_cmpgt_epi32);
	M128i_M128i_M128i(_mm_cmpgt_epi8);
	Ret_M128d_M128d(__m128d, _mm_cmpgt_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpgt_sd);
	Ret_M128d_M128d(__m128d, _mm_cmple_pd);
	Ret_M128d_M128d(__m128d, _mm_cmple_sd);
	M128i_M128i_M128i(_mm_cmplt_epi16);
	M128i_M128i_M128i(_mm_cmplt_epi32);
	M128i_M128i_M128i(_mm_cmplt_epi8);
	Ret_M128d_M128d(__m128d, _mm_cmplt_pd);
	Ret_M128d_M128d(__m128d, _mm_cmplt_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpneq_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpneq_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpnge_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpnge_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpngt_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpngt_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpnle_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpnle_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpnlt_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpnlt_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpord_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpord_sd);
	Ret_M128d_M128d(__m128d, _mm_cmpunord_pd);
	Ret_M128d_M128d(__m128d, _mm_cmpunord_sd);
	Ret_M128d_M128d(int, _mm_comieq_sd);
	Ret_M128d_M128d(int, _mm_comige_sd);
	Ret_M128d_M128d(int, _mm_comigt_sd);
	Ret_M128d_M128d(int, _mm_comile_sd);
	Ret_M128d_M128d(int, _mm_comilt_sd);
	Ret_M128d_M128d(int, _mm_comineq_sd);
	Ret_M128d_M128d(int, _mm_ucomieq_sd);
	Ret_M128d_M128d(int, _mm_ucomige_sd);
	Ret_M128d_M128d(int, _mm_ucomigt_sd);
	Ret_M128d_M128d(int, _mm_ucomile_sd);
	Ret_M128d_M128d(int, _mm_ucomilt_sd);
	Ret_M128d_M128d(int, _mm_ucomineq_sd);
}

void test_convert()
{
	// SSE2 Convert instructions:
	Ret_M128i(__m128d, _mm_cvtepi32_pd);
#ifndef BREAKS_UNDER_OPTIMIZATION
	Ret_M128i(__m128, _mm_cvtepi32_ps);
#endif
	Ret_M128d(__m128i, _mm_cvtpd_epi32);
	Ret_M128d(__m128, _mm_cvtpd_ps);
	Ret_M128(__m128i, _mm_cvtps_epi32);
	Ret_M128(__m128d,  _mm_cvtps_pd);
	Ret_M128(double, _mm_cvtsd_f64);
	Ret_M128d(int, _mm_cvtsd_si32);
#ifndef NO_INT64X2
	Ret_M128d(int64_t, _mm_cvtsd_si64); // _mm_cvtsd_si64x is an alias to this.
#endif
	Ret_M128i(int, _mm_cvtsi128_si32);
#ifndef NO_INT64X2
	Ret_M128i(int64_t, _mm_cvtsi128_si64); // _mm_cvtsi128_si64x is an alias to this.
#endif
	Ret_M128d_int(__m128d, _mm_cvtsi32_sd);
	Ret_int(__m128i, _mm_cvtsi32_si128);
#ifndef NO_INT64X2
	Ret_M128d_int64(__m128d, _mm_cvtsi64_sd); // _mm_cvtsi64x_sd is an alias to this.
	Ret_int64(__m128i, _mm_cvtsi64_si128); // _mm_cvtsi64x_si128 is an alias to this.
#endif
	Ret_M128d_M128d(__m128d, _mm_cvtss_sd);
	Ret_M128d(__m128i, _mm_cvttpd_epi32);
	Ret_M128(__m128i, _mm_cvttps_epi32);
	Ret_M128d(int, _mm_cvttsd_si32);
#ifndef NO_INT64X2
	Ret_M128d(int64_t, _mm_cvttsd_si64); // _mm_cvttsd_si64x is an alias to this.
#endif
}
void test_elementarymath()
{
	// SSE2 Elementary Math Functions instructions:
	Ret_M128d(__m128d, _mm_sqrt_pd);
	Ret_M128d_M128d(__m128d, _mm_sqrt_sd);
}

void test_generalsupport()
{
	// SSE2 General Support instructions:
	_mm_clflush(interesting_floats);
	_mm_lfence();
	_mm_mfence();
	_mm_pause();
}

void test_load()
{
	// SSE2 Load functions:
	Ret_DoublePtr(__m128d, _mm_load_pd, 2, 2);
	Ret_DoublePtr(__m128d, _mm_load_pd1, 1, 1);
	Ret_DoublePtr(__m128d, _mm_load_sd, 1, 1);
#ifndef BREAKS_UNDER_OPTIMIZATION
	Ret_IntPtr(__m128i, _mm_load_si128, __m128i*, 4, 4);
#endif
	Ret_DoublePtr(__m128d, _mm_load1_pd, 1, 1);
	Ret_M128d_DoublePtr(__m128d, _mm_loadh_pd, double*, 1, 1);
	Ret_IntPtr(__m128i, _mm_loadl_epi64, __m128i*, 2, 1);
	Ret_M128d_DoublePtr(__m128d, _mm_loadl_pd, double*, 1, 1);
	Ret_DoublePtr(__m128d, _mm_loadr_pd, 2, 2);
	Ret_DoublePtr(__m128d, _mm_loadu_pd, 2, 1);
#ifndef BREAKS_UNDER_OPTIMIZATION
	Ret_IntPtr(__m128i, _mm_loadu_si128, __m128i*, 4, 1);
#endif
}

void test_logical()
{
	// SSE2 Logical instructions:
	Ret_M128d_M128d(__m128d, _mm_and_pd);
	M128i_M128i_M128i(_mm_and_si128);
	Ret_M128d_M128d(__m128d, _mm_andnot_pd);
	M128i_M128i_M128i(_mm_andnot_si128);
	Ret_M128d_M128d(__m128d, _mm_or_pd);
	M128i_M128i_M128i(_mm_or_si128);
	Ret_M128d_M128d(__m128d, _mm_xor_pd);
	M128i_M128i_M128i(_mm_xor_si128);
}

void test_misc()
{
	// SSE2 Miscellaneous instructions:
#ifndef BREAKS_UNDER_OPTIMIZATION
	Ret_M128i(int, _mm_movemask_epi8);
#endif
	Ret_M128d(int, _mm_movemask_pd);
	M128i_M128i_M128i(_mm_packs_epi16);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_packs_epi32);
#endif
	M128i_M128i_M128i(_mm_packus_epi16);
}

void test_move()
{
	// SSE2 Move instructions:
#ifndef NO_INT64X2
	Ret_M128i(__m128i, _mm_move_epi64);
#endif
	Ret_M128d_M128d(__m128d, _mm_move_sd);
}

void test_probability()
{
	// SSE2 Probability/Statistics instructions:
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_avg_epu16);
#endif
	M128i_M128i_M128i(_mm_avg_epu8);
}

void test_set()
{
/*
	// SSE2 Set functions:
	_mm_set_epi16
	_mm_set_epi32
	_mm_set_epi64
	_mm_set_epi64x
	_mm_set_epi8
	_mm_set_pd
	_mm_set_pd1
	_mm_set_sd
	_mm_set1_epi16
	_mm_set1_epi32
	_mm_set1_epi64
	_mm_set1_epi64x
	_mm_set1_epi8
	_mm_set1_pd
	_mm_setr_epi16
	_mm_setr_epi32
	_mm_setr_epi64
	_mm_setr_epi8
	_mm_setr_pd
	_mm_setzero_pd
	_mm_setzero_si128
*/
}

void test_shift()
{
	// SSE2 Shift instructions:
	M128i_M128i_M128i(_mm_sll_epi16);
	M128i_M128i_M128i(_mm_sll_epi32);
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_sll_epi64);
#endif
	Ret_M128i_Tint(__m128i, _mm_slli_epi16);
	Ret_M128i_Tint(__m128i, _mm_slli_epi32);
#ifndef NO_INT64X2
	Ret_M128i_Tint(__m128i, _mm_slli_epi64);
#endif
	Ret_M128i_Tint(__m128i, _mm_slli_si128); // _mm_bslli_si128 is an alias to this.
	M128i_M128i_M128i(_mm_sra_epi16);
	M128i_M128i_M128i(_mm_sra_epi32);
	Ret_M128i_Tint(__m128i, _mm_srai_epi16);
	Ret_M128i_Tint(__m128i, _mm_srai_epi32);
	M128i_M128i_M128i(_mm_srl_epi16);
	M128i_M128i_M128i(_mm_srl_epi32);
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_srl_epi64);
#endif
#ifndef BREAKS_UNDER_OPTIMIZATION
	Ret_M128i_Tint(__m128i, _mm_srli_epi16);
#endif
	Ret_M128i_Tint(__m128i, _mm_srli_epi32);
#ifndef NO_INT64X2
	Ret_M128i_Tint(__m128i, _mm_srli_epi64);
#endif
	Ret_M128i_Tint(__m128i, _mm_srli_si128); // _mm_bsrli_si128 is an alias to this.
}

void test_specialmath()
{
	// SSE2 Special Math instructions:
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_max_epi16);
	M128i_M128i_M128i(_mm_max_epu8);
#endif
	Ret_M128d_M128d(__m128d, _mm_max_pd);
	Ret_M128d_M128d(__m128d, _mm_max_sd);
#ifndef BREAKS_UNDER_OPTIMIZATION
	M128i_M128i_M128i(_mm_min_epi16);
	M128i_M128i_M128i(_mm_min_epu8);
#endif
	Ret_M128d_M128d(__m128d, _mm_min_pd);
	Ret_M128d_M128d(__m128d, _mm_min_sd);
}

void test_store()
{
	// SSE2 Store instructions:
	void_M128i_M128i_OutIntPtr(_mm_maskmoveu_si128, char*, 16, 1);
	void_OutDoublePtr_M128d(_mm_store_pd, double*, 16, 16);
	void_OutDoublePtr_M128d(_mm_store_sd, double*, 8, 1);
	void_OutIntPtr_M128(_mm_store_si128, __m128i*, 16, 16);
	void_OutDoublePtr_M128d(_mm_store1_pd, double*, 16, 16); // _mm_store_pd1 is an alias to this.
	void_OutDoublePtr_M128d(_mm_storeh_pd, double*, 8, 1);
#ifndef NO_INT64X2
	void_OutIntPtr_M128(_mm_storel_epi64, __m128i*, 8, 1);
#endif
	void_OutDoublePtr_M128d(_mm_storel_pd, double*, 8, 1);
	void_OutDoublePtr_M128d(_mm_storer_pd, double*, 16, 16);
	void_OutDoublePtr_M128d(_mm_storeu_pd, double*, 16, 1);
	void_OutIntPtr_M128(_mm_storeu_si128, __m128i*, 16, 1);
	void_OutDoublePtr_M128d(_mm_stream_pd, double*, 16, 16);
	void_OutIntPtr_M128(_mm_stream_si128, __m128i*, 16, 16);
	void_OutIntPtr_int(_mm_stream_si32, int*, 4, 1);
#ifndef NO_INT64X2
	void_OutIntPtr_int64(_mm_stream_si64, int64_t*, 8, 1);
#endif
}

void test_swizzle()
{
	// SSE2 Swizzle instructions:
	Ret_M128i_Tint(int, _mm_extract_epi16);
	Ret_M128i_int_Tint(__m128i, _mm_insert_epi16);
	Ret_M128i_Tint(__m128i, _mm_shuffle_epi32);
	Ret_M128d_M128d_Tint(__m128d, _mm_shuffle_pd);
	Ret_M128i_Tint(__m128i, _mm_shufflehi_epi16);
	Ret_M128i_Tint(__m128i, _mm_shufflelo_epi16);
	M128i_M128i_M128i(_mm_unpackhi_epi16);
	M128i_M128i_M128i(_mm_unpackhi_epi32);
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_unpackhi_epi64);
#endif
	M128i_M128i_M128i(_mm_unpackhi_epi8);
	Ret_M128d_M128d(__m128d, _mm_unpackhi_pd);
	M128i_M128i_M128i(_mm_unpacklo_epi16);
	M128i_M128i_M128i(_mm_unpacklo_epi32);
#ifndef NO_INT64X2
	M128i_M128i_M128i(_mm_unpacklo_epi64);
#endif
	M128i_M128i_M128i(_mm_unpacklo_epi8);
	Ret_M128d_M128d(__m128d, _mm_unpacklo_pd);
}

int main()
{
	assert(numInterestingFloats % 4 == 0);
	assert(numInterestingInts % 4 == 0);
	assert(numInterestingDoubles % 4 == 0);	

	test_arithmetic();
	test_cast();
	test_compare();
	test_convert();
	test_elementarymath();
	test_generalsupport();
	test_load();
	test_logical();
	test_misc();
	test_move();
	test_probability();
	test_set();
	test_shift();
	test_specialmath();
	test_store();
	test_swizzle();	
}
