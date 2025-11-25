/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses AVX by calling different functions with different interesting
// inputs and prints the results. Use a diff tool to compare the results between
// platforms.

// immintrin.h must be included before test_sse.h
// clang-format off
#include <immintrin.h>
#include "test_sse.h"
// clang-format on

bool testNaNBits = true;

float* interesting_floats = get_interesting_floats();
int numInterestingFloats =
  sizeof(interesting_floats_) / sizeof(interesting_floats_[0]);
uint32_t* interesting_ints = get_interesting_ints();
int numInterestingInts =
  sizeof(interesting_ints_) / sizeof(interesting_ints_[0]);
double* interesting_doubles = get_interesting_doubles();
int numInterestingDoubles =
  sizeof(interesting_doubles_) / sizeof(interesting_doubles_[0]);

void NOINLINE test_add_pd() { testNaNBits = false; Ret_M256d_M256d(__m256d, _mm256_add_pd); }
void NOINLINE test_add_ps() { testNaNBits = false; Ret_M256_M256(__m256, _mm256_add_ps); }

void NOINLINE test_sub_pd() { testNaNBits = true; Ret_M256d_M256d(__m256d, _mm256_sub_pd); }
void NOINLINE test_sub_ps() { testNaNBits = true; Ret_M256_M256(__m256, _mm256_sub_ps); }

void NOINLINE test_addsub_pd() { testNaNBits = false; Ret_M256d_M256d(__m256d, _mm256_addsub_pd); }
void NOINLINE test_addsub_ps() { testNaNBits = false; Ret_M256_M256(__m256, _mm256_addsub_ps); }

void NOINLINE test_div_pd() { testNaNBits = true; Ret_M256d_M256d(__m256d, _mm256_div_pd); }
void NOINLINE test_div_ps() { testNaNBits = true; Ret_M256_M256(__m256, _mm256_div_ps); }

void NOINLINE test_mul_pd() { testNaNBits = false; Ret_M256d_M256d(__m256d, _mm256_mul_pd); }
void NOINLINE test_mul_ps() { testNaNBits = false; Ret_M256_M256(__m256, _mm256_mul_ps); }

void NOINLINE test_hadd_pd() { testNaNBits = false; Ret_M256d_M256d(__m256d, _mm256_hadd_pd); }
void NOINLINE test_hadd_ps() { testNaNBits = false; Ret_M256_M256(__m256, _mm256_hadd_ps); }

void NOINLINE test_hsub_pd() { testNaNBits = true; Ret_M256d_M256d(__m256d, _mm256_hsub_pd); }
void NOINLINE test_hsub_ps() { testNaNBits = true; Ret_M256_M256(__m256, _mm256_hsub_ps); }

void NOINLINE test_dp_ps() { testNaNBits = false; Ret_M256_M256_Tint(__m256, _mm256_dp_ps); }

void NOINLINE test_max_pd() { Ret_M256d_M256d(__m256d, _mm256_max_pd); }
void NOINLINE test_max_ps() { Ret_M256_M256(__m256, _mm256_max_ps); }

void NOINLINE test_min_pd() { Ret_M256d_M256d(__m256d, _mm256_min_pd); }
void NOINLINE test_min_ps() { Ret_M256_M256(__m256, _mm256_min_ps); }

void NOINLINE test_round_pd() { Ret_M256d_Tint(__m256d, _mm256_round_pd); }
void NOINLINE test_round_ps() { Ret_M256_Tint(__m256, _mm256_round_ps); }

void NOINLINE test_ceil_pd() { Ret_M256d(__m256d, _mm256_ceil_pd); }
void NOINLINE test_ceil_ps() { Ret_M256(__m256, _mm256_ceil_ps); }

void NOINLINE test_floor_pd() { Ret_M256d(__m256d, _mm256_floor_pd); }
void NOINLINE test_floor_ps() { Ret_M256(__m256, _mm256_floor_ps); }

void NOINLINE test_sqrt_pd() { Ret_M256d(__m256d, _mm256_sqrt_pd); }
void NOINLINE test_sqrt_ps() { Ret_M256approx(__m256, _mm256_sqrt_ps); }
void NOINLINE test_rsqrt_ps() { Ret_M256approx(__m256, _mm256_rsqrt_ps); }
void NOINLINE test_rcp_ps() { Ret_M256approx(__m256, _mm256_rcp_ps); }

void NOINLINE test_and_pd() { Ret_M128d_M128d(__m128d, _mm_and_pd); }
void NOINLINE test_and_ps() { Ret_M128_M128(__m128, _mm_and_ps); }

void NOINLINE test_andnot_pd() { Ret_M128d_M128d(__m128d, _mm_andnot_pd); }
void NOINLINE test_andnot_ps() { Ret_M128_M128(__m128, _mm_andnot_ps); }

void NOINLINE test_or_pd() { Ret_M128d_M128d(__m128d, _mm_or_pd); }
void NOINLINE test_or_ps() { Ret_M128_M128(__m128, _mm_or_ps); }

void NOINLINE test_xor_pd() { Ret_M128d_M128d(__m128d, _mm_xor_pd); }
void NOINLINE test_xor_ps() { Ret_M128_M128(__m128, _mm_xor_ps); }

void NOINLINE test_testz_pd() { Ret_M128d_M128d(int, _mm_testz_pd); }
void NOINLINE test_testc_pd() { Ret_M128d_M128d(int, _mm_testc_pd); }
void NOINLINE test_testnzc_pd() { Ret_M128d_M128d(int, _mm_testnzc_pd); }

void NOINLINE test_testz_ps() { Ret_M128_M128(int, _mm_testz_ps); }
void NOINLINE test_testc_ps() { Ret_M128_M128(int, _mm_testc_ps); }
void NOINLINE test_testnzc_ps() { Ret_M128_M128(int, _mm_testnzc_ps); }

void NOINLINE test_mm256_testz_pd() { Ret_M256d_M256d(int, _mm256_testz_pd); }
void NOINLINE test_mm256_testc_pd() { Ret_M256d_M256d(int, _mm256_testc_pd); }
void NOINLINE test_mm256_testnzc_pd() { Ret_M256d_M256d(int, _mm256_testnzc_pd); }

void NOINLINE test_mm256_testz_ps() { Ret_M256_M256(int, _mm256_testz_ps); }
void NOINLINE test_mm256_testc_ps() { Ret_M256_M256(int, _mm256_testc_ps); }
void NOINLINE test_mm256_testnzc_ps() { Ret_M256_M256(int, _mm256_testnzc_ps); }

void NOINLINE test_testz_si256() { Ret_M256i_M256i(int, _mm256_testz_si256); }
void NOINLINE test_testc_si256() { Ret_M256i_M256i(int, _mm256_testc_si256); }
void NOINLINE test_testnzc_si256() { Ret_M256i_M256i(int, _mm256_testnzc_si256); }

// split test_swizzle into multiple functions to avoid too many locals error
void NOINLINE test_permutevar_pd() { Ret_M128d_M128i(__m128d, _mm_permutevar_pd); }
void NOINLINE test_permutevar_ps() { Ret_M128_M128i(__m128, _mm_permutevar_ps); }
void NOINLINE test_permute_pd() { Ret_M128d_Tint(__m128d, _mm_permute_pd); }
void NOINLINE test_permute_ps() { Ret_M128_Tint(__m128, _mm_permute_ps); }

void NOINLINE test_mm256_permutevar_pd() { Ret_M256d_M256i(__m256d, _mm256_permutevar_pd); }
void NOINLINE test_mm256_permutevar_ps() { Ret_M256_M256i(__m256, _mm256_permutevar_ps); }
void NOINLINE test_mm256_permute_pd() { Ret_M256d_Tint(__m256d, _mm256_permute_pd); }
void NOINLINE test_mm256_permute_ps() { Ret_M256_Tint(__m256, _mm256_permute_ps); }

void NOINLINE test_permute2f128_pd() { Ret_M256d_M256d_Tint(__m256d, _mm256_permute2f128_pd); }
void NOINLINE test_permute2f128_ps() { Ret_M256_M256_Tint(__m256, _mm256_permute2f128_ps); }
void NOINLINE test_permute2f128_si256() { Ret_M256i_M256i_Tint(__m256i, _mm256_permute2f128_si256); }

void NOINLINE test_blend_pd() { Ret_M256d_M256d_Tint(__m256d, _mm256_blend_pd); }
void NOINLINE test_blend_ps() { Ret_M256_M256_Tint(__m256, _mm256_blend_ps); }
void NOINLINE test_blendv_pd() { Ret_M256d_M256d_M256d(__m256d, _mm256_blendv_pd); }
void NOINLINE test_blendv_ps() { Ret_M256_M256_M256(__m256, _mm256_blendv_ps); }

void NOINLINE test_shuffle_pd() { Ret_M256d_M256d_Tint(__m256d, _mm256_shuffle_pd); }
void NOINLINE test_shuffle_ps() { Ret_M256_M256_Tint(__m256, _mm256_shuffle_ps); }

void NOINLINE test_extract_epi32() { Ret_M256i_Tint(int, _mm256_extract_epi32); }
void NOINLINE test_extract_epi16() { Ret_M256i_Tint(int, _mm256_extract_epi16); }
void NOINLINE test_extract_epi8() { Ret_M256i_Tint(int, _mm256_extract_epi8); }
void NOINLINE test_extract_epi64() { Ret_M256i_Tint(int64_t, _mm256_extract_epi64); }

void NOINLINE test_insert_epi32() { Ret_M256i_int_Tint(__m256i, _mm256_insert_epi32); }
void NOINLINE test_insert_epi16() { Ret_M256i_int_Tint(__m256i, _mm256_insert_epi16); }
void NOINLINE test_insert_epi8() { Ret_M256i_int_Tint(__m256i, _mm256_insert_epi8); }
void NOINLINE test_insert_epi64() { Ret_M256i_int_Tint(__m256i, _mm256_insert_epi64); }

void NOINLINE test_unpackhi_pd() { Ret_M256d_M256d(__m256d, _mm256_unpackhi_pd); }
void NOINLINE test_unpacklo_pd() { Ret_M256d_M256d(__m256d, _mm256_unpacklo_pd); }
void NOINLINE test_unpackhi_ps() { Ret_M256_M256(__m256, _mm256_unpackhi_ps); }
void NOINLINE test_unpacklo_ps() { Ret_M256_M256(__m256, _mm256_unpacklo_ps); }

void NOINLINE test_insertf128_pd() { Ret_M256d_M128d_Tint(__m256d, _mm256_insertf128_pd); }
void NOINLINE test_insertf128_ps() { Ret_M256_M128_Tint(__m256, _mm256_insertf128_ps); }
void NOINLINE test_insertf128_si256() { Ret_M256i_M128i_Tint(__m256i, _mm256_insertf128_si256); }

void NOINLINE test_extractf128_pd() { Ret_M256d_Tint(__m128d, _mm256_extractf128_pd); }
void NOINLINE test_extractf128_ps() { Ret_M256_Tint(__m128, _mm256_extractf128_ps); }
void NOINLINE test_extractf128_si256() { Ret_M256i_Tint(__m128i, _mm256_extractf128_si256); }

void NOINLINE test_cvtepi32_pd() { Ret_M128i(__m256d, _mm256_cvtepi32_pd); }
void NOINLINE test_cvtepi32_ps() { Ret_M256i(__m256, _mm256_cvtepi32_ps); }

void NOINLINE test_cvtpd_ps() { Ret_M256d(__m128, _mm256_cvtpd_ps); }

void NOINLINE test_cvtps_epi32() { Ret_M256(__m256i, _mm256_cvtps_epi32); }
void NOINLINE test_cvtps_pd() { Ret_M128(__m256d, _mm256_cvtps_pd); }

void NOINLINE test_cvttpd_epi32() { Ret_M256d(__m128i, _mm256_cvttpd_epi32); }

void NOINLINE test_cvtpd_epi32() { Ret_M256d(__m128i, _mm256_cvtpd_epi32); }

void NOINLINE test_cvttps_epi32() { Ret_M256(__m256i, _mm256_cvttps_epi32); }

void NOINLINE test_cvtsd_f64() { Ret_M256d(double, _mm256_cvtsd_f64); }

void NOINLINE test_cvtsi256_si32() { Ret_M256i(int, _mm256_cvtsi256_si32); }

void NOINLINE test_cvtss_f32() { Ret_M256(float, _mm256_cvtss_f32); }

void NOINLINE test_movehdup_ps() { Ret_M256(__m256, _mm256_movehdup_ps); }
void NOINLINE test_moveldup_ps() { Ret_M256(__m256, _mm256_moveldup_ps); }
void NOINLINE test_movedup_pd() { Ret_M256d(__m256d, _mm256_movedup_pd); }

// split test_compare into multiple functions to avoid too many locals error
void NOINLINE test_cmp_pd() { Ret_M128d_M128d_Tint_5bits(__m128d, _mm_cmp_pd); }
void NOINLINE test_cmp_ps() { Ret_M128_M128_Tint_5bits(__m128, _mm_cmp_ps); }
void NOINLINE test_cmp_sd() { Ret_M128d_M128d_Tint_5bits(__m128d, _mm_cmp_sd); }
void NOINLINE test_cmp_ss() { Ret_M128_M128_Tint_5bits(__m128, _mm_cmp_ss); }

void NOINLINE test_mm256_cmp_pd_0_15() { Ret_M256d_M256d_Tint_5bits_0_to_15(__m256d, _mm256_cmp_pd); }
void NOINLINE test_mm256_cmp_pd_16_31() { Ret_M256d_M256d_Tint_5bits_16_to_31(__m256d, _mm256_cmp_pd); }

void NOINLINE test_mm256_cmp_ps_0_15() { Ret_M256_M256_Tint_5bits_0_to_15(__m256, _mm256_cmp_ps); }
void NOINLINE test_mm256_cmp_ps_16_31() { Ret_M256_M256_Tint_5bits_16_to_31(__m256, _mm256_cmp_ps); }

void NOINLINE test_movemask_pd() { Ret_M256d(int, _mm256_movemask_pd); }
void NOINLINE test_movemask_ps() { Ret_M256(int, _mm256_movemask_ps); }

void NOINLINE test_broadcast_ss() { Ret_FloatPtr(__m128, _mm_broadcast_ss, 1, 1); }
void NOINLINE test_mm256_broadcast_sd() { Ret_DoublePtr(__m256d, _mm256_broadcast_sd, 1, 1); }
void NOINLINE test_mm256_broadcast_ss() { Ret_FloatPtr(__m256, _mm256_broadcast_ss, 1, 1); }

// TODO reuse Ret_DoublePtr?
void NOINLINE test_broadcast_pd() { Ret_M128dPtr(__m256d, _mm256_broadcast_pd); }
void NOINLINE test_broadcast_ps() { Ret_M128Ptr(__m256, _mm256_broadcast_ps); } // must aligned? not sure

void NOINLINE test_load_pd() { Ret_DoublePtr(__m256d, _mm256_load_pd, 4, 4); } // error, input not aligned
void NOINLINE test_load_ps() { Ret_FloatPtr(__m256, _mm256_load_ps, 8, 8); }   // error, align
void NOINLINE test_loadu_pd() { Ret_DoublePtr(__m256d, _mm256_loadu_pd, 4, 1); }
void NOINLINE test_loadu_ps() { Ret_FloatPtr(__m256, _mm256_loadu_ps, 8, 1); }

void NOINLINE test_load_si256() { Ret_IntPtr(__m256i, _mm256_load_si256, __m256i*, 8, 8); } // error, align
void NOINLINE test_loadu_si256() { Ret_IntPtr(__m256i, _mm256_loadu_si256, __m256i_u*, 8, 1); }
void NOINLINE test_lddqu_si256() { Ret_IntPtr(__m256i, _mm256_lddqu_si256, __m256i_u*, 8, 1); }

void NOINLINE test_maskload_pd() { Ret_DoublePtr_M128i(__m128d, _mm_maskload_pd, 2, 2); }
void NOINLINE test_mm256_maskload_pd() { Ret_DoublePtr_M256i(__m256d, _mm256_maskload_pd, 4, 4); }
void NOINLINE test_maskload_ps() { Ret_FloatPtr_M128i(__m128, _mm_maskload_ps, 4, 4); }
void NOINLINE test_mm256_maskload_ps() { Ret_FloatPtr_M256i(__m256, _mm256_maskload_ps, 8, 8); }

void NOINLINE test_loadu2_m128d() { Ret_DoublePtr_DoublePtr(__m256d, _mm256_loadu2_m128d, 2, 2); }
void NOINLINE test_loadu2_m128() { Ret_FloatPtr_FloatPtr(__m256, _mm256_loadu2_m128, 4, 4); }
void NOINLINE test_loadu2_m128i() { Ret_IntPtr_IntPtr(__m256i, _mm256_loadu2_m128i, __m128i_u*, 4, 4); }

void NOINLINE test_store_pd() { void_OutDoublePtr_M256d(_mm256_store_pd, double*, 32, 32); }
void NOINLINE test_store_ps() { void_OutFloatPtr_M256(_mm256_store_ps, float*, 32, 32); }
void NOINLINE test_storeu_pd() { void_OutDoublePtr_M256d(_mm256_storeu_pd, double*, 32, 1); }
void NOINLINE test_storeu_ps() { void_OutFloatPtr_M256(_mm256_storeu_ps, float*, 32, 1); }
void NOINLINE test_store_si256() { void_OutIntPtr_M256i(_mm256_store_si256, __m256i*, 32, 32); }
void NOINLINE test_storeu_si256() { void_OutIntPtr_M256i(_mm256_storeu_si256, __m256i_u*, 32, 1); }

void NOINLINE test_maskstore_pd() { void_OutDoublePtr_M128i_M128d(_mm_maskstore_pd, double*, 16, 8); }
void NOINLINE test_mm256_maskstore_pd() { void_OutDoublePtr_M256i_M256d(_mm256_maskstore_pd, double*, 32, 8); }
void NOINLINE test_maskstore_ps() { void_OutFloatPtr_M128i_M128(_mm_maskstore_ps, float*, 16, 4); }
void NOINLINE test_mm256_maskstore_ps() { void_OutFloatPtr_M256i_M256(_mm256_maskstore_ps, float*, 32, 4); }

void NOINLINE test_stream_si256() { void_OutIntPtr_M256i(_mm256_stream_si256, __m256i*, 32, 32); }
void NOINLINE test_stream_pd() { void_OutDoublePtr_M256d(_mm256_stream_pd, double*, 32, 32); }
void NOINLINE test_stream_ps() { void_OutFloatPtr_M256(_mm256_stream_ps, float*, 32, 32); }

void NOINLINE test_storeu2_m128() { void_OutFloatPtr_OutFloatPtr_M256(_mm256_storeu2_m128, float*, 32, 1); }
void NOINLINE test_storeu2_m128d() { void_OutDoublePtr_OutDoublePtr_M256d(_mm256_storeu2_m128d, double*, 32, 1); }
void NOINLINE test_storeu2_m128i() { void_OutIntPtr_OutIntPtr_M256i(_mm256_storeu2_m128i, __m128i_u*, 32, 1) }

void test_undef(void) {
#ifdef __EMSCRIPTEN__
  _mm256_undefined_pd();
  _mm256_undefined_ps();
  _mm256_undefined_si256();
#endif
}

void NOINLINE test_set_pd() { Ret_Double4(__m256d, _mm256_set_pd, 1); }
void NOINLINE test_set_ps() { Ret_Float8(__m256, _mm256_set_ps, 1); }
void NOINLINE test_set_epi32() { Ret_Int8(__m256i, _mm256_set_epi32, 1); }
void NOINLINE test_set_epi16() { Ret_Short16(__m256i, _mm256_set_epi16, 2); }
void NOINLINE test_set_epi8() { Ret_Char32(__m256i, _mm256_set_epi8, 4); }
void NOINLINE test_set_epi64x() { Ret_Longlong4(__m256i, _mm256_set_epi64x, 1); }

void NOINLINE test_setr_pd() { Ret_Double4(__m256d, _mm256_setr_pd, 1); }
void NOINLINE test_setr_ps() { Ret_Float8(__m256, _mm256_setr_ps, 1); }
void NOINLINE test_setr_epi32() { Ret_Int8(__m256i, _mm256_setr_epi32, 1); }
void NOINLINE test_setr_epi16() { Ret_Short16(__m256i, _mm256_setr_epi16, 2); }
void NOINLINE test_setr_epi8() { Ret_Char32(__m256i, _mm256_setr_epi8, 4); }
void NOINLINE test_setr_epi64x() { Ret_Longlong4(__m256i, _mm256_setr_epi64x, 1); }

void NOINLINE test_set1_pd() { Ret_Double(__m256d, _mm256_set1_pd, 1); }
void NOINLINE test_set1_ps() { Ret_Float(__m256, _mm256_set1_ps, 1); }
void NOINLINE test_set1_epi32() { Ret_Int(__m256i, _mm256_set1_epi32, 1); }
void NOINLINE test_set1_epi16() { Ret_Int(__m256i, _mm256_set1_epi16, 1); }
void NOINLINE test_set1_epi8() { Ret_Int(__m256i, _mm256_set1_epi8, 1); }
void NOINLINE test_set1_epi64x() { Ret_Int(__m256i, _mm256_set1_epi64x, 1); }

void NOINLINE test_setzero_pd() { 
  char str[256] = {};
  __m256d zerod = _mm256_setzero_pd();
  tostr(&zerod, str);
  printf("_mm256_setzero_pd() = %s\n", str);
}

void NOINLINE test_setzero_ps() { 
  char str[256] = {};
  __m256 zero = _mm256_setzero_ps();
  tostr(&zero, str);
  printf("_mm256_setzero_ps() = %s\n", str);
}

void NOINLINE test_setzero_si256() { 
  char str[256] = {};
  __m256i zeroi = _mm256_setzero_si256();
  tostr(&zeroi, str);
  printf("_mm256_setzero_si256() = %s\n", str);
}

void NOINLINE test_set_m128() { Ret_M128_M128(__m256, _mm256_set_m128); }
void NOINLINE test_set_m128d() { Ret_M128d_M128d(__m256d, _mm256_set_m128d); }
void NOINLINE test_set_m128i() { Ret_M128i_M128i(__m256i, _mm256_set_m128i); }

void NOINLINE test_setr_m128() { Ret_M128_M128(__m256, _mm256_setr_m128); }
void NOINLINE test_setr_m128d() { Ret_M128d_M128d(__m256d, _mm256_setr_m128d); }
void NOINLINE test_setr_m128i() { Ret_M128i_M128i(__m256i, _mm256_setr_m128i); }

void NOINLINE test_castpd_ps() { Ret_M256d(__m256, _mm256_castpd_ps); }
void NOINLINE test_castpd_si256() { Ret_M256d(__m256i, _mm256_castpd_si256); }
void NOINLINE test_castps_pd() { Ret_M256(__m256d, _mm256_castps_pd); }
void NOINLINE test_castps_si256() { Ret_M256(__m256i, _mm256_castps_si256); }
void NOINLINE test_castsi256_pd() { Ret_M256i(__m256d, _mm256_castsi256_pd); }
void NOINLINE test_castsi256_ps() { Ret_M256i(__m256, _mm256_castsi256_ps); }

void NOINLINE test_castpd256_pd128() { Ret_M256d(__m128d, _mm256_castpd256_pd128); }
void NOINLINE test_castps256_ps128() { Ret_M256(__m128, _mm256_castps256_ps128); }
void NOINLINE test_castsi256_si128() { Ret_M256i(__m128i, _mm256_castsi256_si128); }
void NOINLINE test_castpd128_pd256() { Ret_M128d(__m256d, _mm256_castpd128_pd256); }
void NOINLINE test_castps128_ps256() { Ret_M128(__m256, _mm256_castps128_ps256); }
void NOINLINE test_castsi128_si256() { Ret_M128i(__m256i, _mm256_castsi128_si256); }

void NOINLINE test_zextpd128_pd256() { Ret_M128d(__m256d, _mm256_zextpd128_pd256); }
void NOINLINE test_zextps128_ps256() { Ret_M128(__m256, _mm256_zextps128_ps256); }
void NOINLINE test_zextsi128_si256() { Ret_M128i(__m256i, _mm256_zextsi128_si256); }

int main() {
  assert(numInterestingFloats % 8 == 0);
  assert(numInterestingInts % 8 == 0);
  assert(numInterestingDoubles % 4 == 0);

  test_add_pd();
  test_add_ps();
  test_sub_pd();
  test_sub_ps();
  test_addsub_pd();
  test_addsub_ps();
  test_div_pd();
  test_div_ps();
  test_mul_pd();
  test_mul_ps();
  test_hadd_pd();
  test_hadd_ps();
  test_hsub_pd();
  test_hsub_ps();
  test_dp_ps();

  testNaNBits = false;

  test_max_pd();
  test_max_ps();
  test_min_pd();
  test_min_ps();
  test_round_pd();
  test_round_ps();
  test_ceil_pd();
  test_ceil_ps();
  test_floor_pd();
  test_floor_ps();

  test_sqrt_pd();
  test_sqrt_ps();
  test_rsqrt_ps(); 
  test_rcp_ps();

  test_and_pd();
  test_and_ps();
  test_andnot_pd();
  test_andnot_ps();
  test_or_pd();
  test_or_ps();
  test_xor_pd();
  test_xor_ps();
  test_testz_pd();
  test_testc_pd();
  test_testnzc_pd();
  test_testz_ps();
  test_testc_ps();
  test_testnzc_ps();
  test_mm256_testz_pd();
  test_mm256_testc_pd();
  test_mm256_testnzc_pd();
  test_mm256_testz_ps();
  test_mm256_testc_ps();
  test_mm256_testnzc_ps();
  test_testz_si256();
  test_testc_si256();
  test_testnzc_si256();

  test_permutevar_pd();
  test_permutevar_ps();
  test_permute_pd();
  test_permute_ps();
  test_mm256_permutevar_pd();
  test_mm256_permutevar_ps();
  test_mm256_permute_pd();
  test_mm256_permute_ps();
  test_permute2f128_pd();
  test_permute2f128_ps();
  test_permute2f128_si256();

  test_blend_pd();
  test_blend_ps();
  test_blendv_pd();
  test_blendv_ps();

  test_shuffle_pd();
  test_shuffle_ps();

  test_extract_epi32();
  test_extract_epi16();
  test_extract_epi8();
  test_extract_epi64();

  test_insert_epi32();
  test_insert_epi16();
  test_insert_epi8();
  test_insert_epi64();
  test_unpackhi_pd();
  test_unpacklo_pd();
  test_unpackhi_ps();
  test_unpacklo_ps();
  test_insertf128_pd();
  test_insertf128_ps();
  test_insertf128_si256();
  test_extractf128_pd();
  test_extractf128_ps();
  test_extractf128_si256();

  test_cvtepi32_pd();
  test_cvtepi32_ps();
  test_cvtpd_ps();
  test_cvtps_epi32();
  test_cvtps_pd();
  test_cvttpd_epi32();
  test_cvtpd_epi32();
  test_cvttps_epi32();
  test_cvtsd_f64();
  test_cvtsi256_si32();
  test_cvtss_f32();

  test_movehdup_ps();
  test_moveldup_ps();
  test_movedup_pd();

  test_cmp_pd();
  test_cmp_ps();
  test_cmp_sd();
  test_cmp_ss();

  test_mm256_cmp_pd_0_15();
  test_mm256_cmp_pd_16_31();

  test_mm256_cmp_ps_0_15();
  test_mm256_cmp_ps_16_31();

  test_movemask_pd();
  test_movemask_ps();

  test_broadcast_ss();
  test_mm256_broadcast_sd();
  test_mm256_broadcast_ss();

  test_broadcast_pd();
  test_broadcast_ps();

  test_load_pd();
  test_load_ps();
  test_loadu_pd();
  test_loadu_ps();

  test_load_si256();
  test_loadu_si256();
  test_lddqu_si256();

  test_maskload_pd();
  test_mm256_maskload_pd();
  test_maskload_ps();
  test_mm256_maskload_ps();

  test_loadu2_m128d();
  test_loadu2_m128();
  test_loadu2_m128i();

  test_store_pd();
  test_store_ps();
  test_storeu_pd();
  test_storeu_ps();
  test_store_si256();
  test_storeu_si256();

  test_maskstore_pd();
  test_mm256_maskstore_pd();
  test_maskstore_ps();
  test_mm256_maskstore_ps();

  test_stream_si256();
  test_stream_pd();
  test_stream_ps();

  test_storeu2_m128();
  test_storeu2_m128d();
  test_storeu2_m128i();

  test_undef();

  test_set_pd();
  test_set_ps();
  test_set_epi32();
  test_set_epi16();
  test_set_epi8();
  test_set_epi64x();

  test_setr_pd();
  test_setr_ps();
  test_setr_epi32();
  test_setr_epi16();
  test_setr_epi8();
  test_setr_epi64x();

  test_set1_pd();
  test_set1_ps();
  test_set1_epi32();
  test_set1_epi16();
  test_set1_epi8();
  test_set1_epi64x();

  test_setzero_pd();

  test_setzero_ps();

  test_setzero_si256();

  test_set_m128();
  test_set_m128d();
  test_set_m128i();

  test_setr_m128();
  test_setr_m128d();
  test_setr_m128i();

  test_castpd_ps();
  test_castpd_si256();
  test_castps_pd();
  test_castps_si256();
  test_castsi256_pd();
  test_castsi256_ps();

  test_castpd256_pd128();
  test_castps256_ps128();
  test_castsi256_si128();
  test_castpd128_pd256();
  test_castps128_ps256();
  test_castsi128_si256();

  test_zextpd128_pd256();
  test_zextps128_ps256();
  test_zextsi128_si256();
}
