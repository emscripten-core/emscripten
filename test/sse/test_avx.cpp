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

void test_arithmetic(void) {
  testNaNBits = false;
  Ret_M256d_M256d(__m256d, _mm256_add_pd);
  Ret_M256_M256(__m256, _mm256_add_ps);

  testNaNBits = true;
  Ret_M256d_M256d(__m256d, _mm256_sub_pd);
  Ret_M256_M256(__m256, _mm256_sub_ps);

  testNaNBits = false;
  Ret_M256d_M256d(__m256d, _mm256_addsub_pd);
  Ret_M256_M256(__m256, _mm256_addsub_ps);

  testNaNBits = true;
  Ret_M256d_M256d(__m256d, _mm256_div_pd);
  Ret_M256_M256(__m256, _mm256_div_ps);

  testNaNBits = false;
  Ret_M256d_M256d(__m256d, _mm256_mul_pd);
  Ret_M256_M256(__m256, _mm256_mul_ps);

  Ret_M256d_M256d(__m256d, _mm256_hadd_pd);
  Ret_M256_M256(__m256, _mm256_hadd_ps);

  testNaNBits = true;
  Ret_M256d_M256d(__m256d, _mm256_hsub_pd);
  Ret_M256_M256(__m256, _mm256_hsub_ps);

  testNaNBits = false;
  Ret_M256_M256_Tint(__m256, _mm256_dp_ps);
}

void test_special_math(void) {
  Ret_M256d_M256d(__m256d, _mm256_max_pd);
  Ret_M256_M256(__m256, _mm256_max_ps);

  Ret_M256d_M256d(__m256d, _mm256_min_pd);
  Ret_M256_M256(__m256, _mm256_min_ps);

  Ret_M256d_Tint(__m256d, _mm256_round_pd);
  Ret_M256_Tint(__m256, _mm256_round_ps);

  Ret_M256d(__m256d, _mm256_ceil_pd);
  Ret_M256(__m256, _mm256_ceil_ps);

  Ret_M256d(__m256d, _mm256_floor_pd);
  Ret_M256(__m256, _mm256_floor_ps);
}

void test_elementary_math(void) {
  Ret_M256d(__m256d, _mm256_sqrt_pd);
  Ret_M256approx(__m256, _mm256_sqrt_ps);
  Ret_M256approx(__m256, _mm256_rsqrt_ps);
  Ret_M256approx(__m256, _mm256_rcp_ps);
}

void test_logical(void) {
  Ret_M128d_M128d(__m128d, _mm_and_pd);
  Ret_M128_M128(__m128, _mm_and_ps);

  Ret_M128d_M128d(__m128d, _mm_andnot_pd);
  Ret_M128_M128(__m128, _mm_andnot_ps);

  Ret_M128d_M128d(__m128d, _mm_or_pd);
  Ret_M128_M128(__m128, _mm_or_ps);

  Ret_M128d_M128d(__m128d, _mm_xor_pd);
  Ret_M128_M128(__m128, _mm_xor_ps);

  Ret_M128d_M128d(int, _mm_testz_pd);
  Ret_M128d_M128d(int, _mm_testc_pd);
  Ret_M128d_M128d(int, _mm_testnzc_pd);

  Ret_M128_M128(int, _mm_testz_ps);
  Ret_M128_M128(int, _mm_testc_ps);
  Ret_M128_M128(int, _mm_testnzc_ps);

  Ret_M256d_M256d(int, _mm256_testz_pd);
  Ret_M256d_M256d(int, _mm256_testc_pd);
  Ret_M256d_M256d(int, _mm256_testnzc_pd);

  Ret_M256_M256(int, _mm256_testz_ps);
  Ret_M256_M256(int, _mm256_testc_ps);
  Ret_M256_M256(int, _mm256_testnzc_ps);

  Ret_M256i_M256i(int, _mm256_testz_si256);
  Ret_M256i_M256i(int, _mm256_testc_si256);
  Ret_M256i_M256i(int, _mm256_testnzc_si256);
}

// split test_swizzle into multiple functions to avoid too many locals error
void test_swizzle_128bit() {
  Ret_M128d_M128i(__m128d, _mm_permutevar_pd);
  Ret_M128_M128i(__m128, _mm_permutevar_ps);
  Ret_M128d_Tint(__m128d, _mm_permute_pd);
  Ret_M128_Tint(__m128, _mm_permute_ps);
}

void test_swizzle_permute2f128() {
  Ret_M256d_M256i(__m256d, _mm256_permutevar_pd);
  Ret_M256_M256i(__m256, _mm256_permutevar_ps);
  Ret_M256d_Tint(__m256d, _mm256_permute_pd);
  Ret_M256_Tint(__m256, _mm256_permute_ps);

  Ret_M256d_M256d_Tint(__m256d, _mm256_permute2f128_pd);
  Ret_M256_M256_Tint(__m256, _mm256_permute2f128_ps);
  Ret_M256i_M256i_Tint(__m256i, _mm256_permute2f128_si256);
}

void test_swizzle_blend() {
  Ret_M256d_M256d_Tint(__m256d, _mm256_blend_pd);
  Ret_M256_M256_Tint(__m256, _mm256_blend_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_blendv_pd);
  Ret_M256_M256_M256(__m256, _mm256_blendv_ps);
}

void test_swizzle_shuffle() {
  Ret_M256d_M256d_Tint(__m256d, _mm256_shuffle_pd);
  Ret_M256_M256_Tint(__m256, _mm256_shuffle_ps);
}

void test_swizzle_extract_int() {
  Ret_M256i_Tint(int, _mm256_extract_epi32);
  Ret_M256i_Tint(int, _mm256_extract_epi16);
  Ret_M256i_Tint(int, _mm256_extract_epi8);
  Ret_M256i_Tint(int64_t, _mm256_extract_epi64);
}

void test_swizzle_insert_int() {
  Ret_M256i_int_Tint(__m256i, _mm256_insert_epi32);
  Ret_M256i_int_Tint(__m256i, _mm256_insert_epi16);
  Ret_M256i_int_Tint(__m256i, _mm256_insert_epi8);
  Ret_M256i_int_Tint(__m256i, _mm256_insert_epi64);
}

void test_swizzle_unpack() {
  Ret_M256d_M256d(__m256d, _mm256_unpackhi_pd);
  Ret_M256d_M256d(__m256d, _mm256_unpacklo_pd);
  Ret_M256_M256(__m256, _mm256_unpackhi_ps);
  Ret_M256_M256(__m256, _mm256_unpacklo_ps);
}

void test_swizzle_insertf128() {
  Ret_M256d_M128d_Tint(__m256d, _mm256_insertf128_pd);
  Ret_M256_M128_Tint(__m256, _mm256_insertf128_ps);
  Ret_M256i_M128i_Tint(__m256i, _mm256_insertf128_si256);
}

void test_swizzle_extractf128() {
  Ret_M256d_Tint(__m128d, _mm256_extractf128_pd);
  Ret_M256_Tint(__m128, _mm256_extractf128_ps);
  Ret_M256i_Tint(__m128i, _mm256_extractf128_si256);
}

void test_swizzle(void) {
  test_swizzle_128bit();
  test_swizzle_permute2f128();
  test_swizzle_blend();
  test_swizzle_shuffle();
  test_swizzle_extract_int();
  test_swizzle_insert_int();
  test_swizzle_unpack();
  test_swizzle_insertf128();
  test_swizzle_extractf128();
}

void test_convert(void) {
  Ret_M128i(__m256d, _mm256_cvtepi32_pd);
  Ret_M256i(__m256, _mm256_cvtepi32_ps);

  Ret_M256d(__m128, _mm256_cvtpd_ps);

  Ret_M256(__m256i, _mm256_cvtps_epi32);
  Ret_M128(__m256d, _mm256_cvtps_pd);

  Ret_M256d(__m128i, _mm256_cvttpd_epi32);

  Ret_M256d(__m128i, _mm256_cvtpd_epi32);

  Ret_M256(__m256i, _mm256_cvttps_epi32);

  Ret_M256d(double, _mm256_cvtsd_f64);

  Ret_M256i(int, _mm256_cvtsi256_si32);

  Ret_M256(float, _mm256_cvtss_f32);
}

void test_move(void) {
  Ret_M256(__m256, _mm256_movehdup_ps);
  Ret_M256(__m256, _mm256_moveldup_ps);
  Ret_M256d(__m256d, _mm256_movedup_pd);
}

// split test_compare into multiple functions to avoid too many locals error
void test_compare_128bit() {
  Ret_M128d_M128d_Tint_5bits(__m128d, _mm_cmp_pd);
  Ret_M128_M128_Tint_5bits(__m128, _mm_cmp_ps);
  Ret_M128d_M128d_Tint_5bits(__m128d, _mm_cmp_sd);
  Ret_M128_M128_Tint_5bits(__m128, _mm_cmp_ss);
}

void test_mm256_cmp_pd_tint_0_to_15() {
  Ret_M256d_M256d_Tint_5bits_0_to_15(__m256d, _mm256_cmp_pd);
}

void test_mm256_cmp_pd_tint_16_to_31() {
  Ret_M256d_M256d_Tint_5bits_16_to_31(__m256d, _mm256_cmp_pd);
}

void test_mm256_cmp_pd() {
  test_mm256_cmp_pd_tint_0_to_15();
  test_mm256_cmp_pd_tint_16_to_31();
}

void test_mm256_cmp_ps_tint_0_to_15() {
  Ret_M256_M256_Tint_5bits_0_to_15(__m256, _mm256_cmp_ps);
}

void test_mm256_cmp_ps_tint_16_to_31() {
  Ret_M256_M256_Tint_5bits_16_to_31(__m256, _mm256_cmp_ps);
}

void test_mm256_cmp_ps() {
  test_mm256_cmp_ps_tint_0_to_15();
  test_mm256_cmp_ps_tint_16_to_31();
}

void test_compare(void) {
  test_compare_128bit();
  test_mm256_cmp_pd();
  test_mm256_cmp_ps();
}

void test_misc(void) {
  Ret_M256d(int, _mm256_movemask_pd);
  Ret_M256(int, _mm256_movemask_ps);
}

void test_load(void) {
  Ret_FloatPtr(__m128, _mm_broadcast_ss, 1, 1);
  Ret_DoublePtr(__m256d, _mm256_broadcast_sd, 1, 1);
  Ret_FloatPtr(__m256, _mm256_broadcast_ss, 1, 1);

  // TODO reuse Ret_DoublePtr?
  Ret_M128dPtr(__m256d, _mm256_broadcast_pd);
  Ret_M128Ptr(__m256, _mm256_broadcast_ps); // must aligned? not sure

  Ret_DoublePtr(__m256d, _mm256_load_pd, 4, 4); // error, input not aligned
  Ret_FloatPtr(__m256, _mm256_load_ps, 8, 8);   // error, align
  Ret_DoublePtr(__m256d, _mm256_loadu_pd, 4, 1);
  Ret_FloatPtr(__m256, _mm256_loadu_ps, 8, 1);

  Ret_IntPtr(__m256i, _mm256_load_si256, __m256i*, 8, 8); // error, align
  Ret_IntPtr(__m256i, _mm256_loadu_si256, __m256i_u*, 8, 1);
  Ret_IntPtr(__m256i, _mm256_lddqu_si256, __m256i_u*, 8, 1);

  Ret_DoublePtr_M128i(__m128d, _mm_maskload_pd, 2, 2);
  Ret_DoublePtr_M256i(__m256d, _mm256_maskload_pd, 4, 4);
  Ret_FloatPtr_M128i(__m128, _mm_maskload_ps, 4, 4);
  Ret_FloatPtr_M256i(__m256, _mm256_maskload_ps, 8, 8);

  Ret_DoublePtr_DoublePtr(__m256d, _mm256_loadu2_m128d, 2, 2);
  Ret_FloatPtr_FloatPtr(__m256, _mm256_loadu2_m128, 4, 4);
  Ret_IntPtr_IntPtr(__m256i, _mm256_loadu2_m128i, __m128i_u*, 4, 4);
}

void test_store(void) {

  void_OutDoublePtr_M256d(_mm256_store_pd, double*, 32, 32);
  void_OutFloatPtr_M256(_mm256_store_ps, float*, 32, 32);
  void_OutDoublePtr_M256d(_mm256_storeu_pd, double*, 32, 1);
  void_OutFloatPtr_M256(_mm256_storeu_ps, float*, 32, 1);
  void_OutIntPtr_M256i(_mm256_store_si256, __m256i*, 32, 32);
  void_OutIntPtr_M256i(_mm256_storeu_si256, __m256i_u*, 32, 1);

  void_OutDoublePtr_M128i_M128d(_mm_maskstore_pd, double*, 16, 8);
  void_OutDoublePtr_M256i_M256d(_mm256_maskstore_pd, double*, 32, 8);
  void_OutFloatPtr_M128i_M128(_mm_maskstore_ps, float*, 16, 4);
  void_OutFloatPtr_M256i_M256(_mm256_maskstore_ps, float*, 32, 4);

  void_OutIntPtr_M256i(_mm256_stream_si256, __m256i*, 32, 32);
  void_OutDoublePtr_M256d(_mm256_stream_pd, double*, 32, 32);
  void_OutFloatPtr_M256(_mm256_stream_ps, float*, 32, 32);

  void_OutFloatPtr_OutFloatPtr_M256(_mm256_storeu2_m128, float*, 32, 1);
  void_OutDoublePtr_OutDoublePtr_M256d(_mm256_storeu2_m128d, double*, 32, 1);
  void_OutIntPtr_OutIntPtr_M256i(_mm256_storeu2_m128i, __m128i_u*, 32, 1)
}

void test_undef(void) {
#ifdef __EMSCRIPTEN__
  _mm256_undefined_pd();
  _mm256_undefined_ps();
  _mm256_undefined_si256();
#endif
}

void test_set(void) {
  Ret_Double4(__m256d, _mm256_set_pd, 1);
  Ret_Float8(__m256, _mm256_set_ps, 1);
  Ret_Int8(__m256i, _mm256_set_epi32, 1);
  Ret_Short16(__m256i, _mm256_set_epi16, 2);
  Ret_Char32(__m256i, _mm256_set_epi8, 4);
  Ret_Longlong4(__m256i, _mm256_set_epi64x, 1);

  Ret_Double4(__m256d, _mm256_setr_pd, 1);
  Ret_Float8(__m256, _mm256_setr_ps, 1);
  Ret_Int8(__m256i, _mm256_setr_epi32, 1);
  Ret_Short16(__m256i, _mm256_setr_epi16, 2);
  Ret_Char32(__m256i, _mm256_setr_epi8, 4);
  Ret_Longlong4(__m256i, _mm256_setr_epi64x, 1);

  Ret_Double(__m256d, _mm256_set1_pd, 1);
  Ret_Float(__m256, _mm256_set1_ps, 1);
  Ret_Int(__m256i, _mm256_set1_epi32, 1);
  Ret_Int(__m256i, _mm256_set1_epi16, 1);
  Ret_Int(__m256i, _mm256_set1_epi8, 1);
  Ret_Int(__m256i, _mm256_set1_epi64x, 1);

  char str[256] = {};
  __m256d zerod = _mm256_setzero_pd();
  tostr(&zerod, str);
  printf("_mm256_setzero_pd() = %s\n", str);

  __m256 zero = _mm256_setzero_ps();
  tostr(&zero, str);
  printf("_mm256_setzero_ps() = %s\n", str);

  __m256i zeroi = _mm256_setzero_si256();
  tostr(&zeroi, str);
  printf("_mm256_setzero_si256() = %s\n", str);

  Ret_M128_M128(__m256, _mm256_set_m128);
  Ret_M128d_M128d(__m256d, _mm256_set_m128d);
  Ret_M128i_M128i(__m256i, _mm256_set_m128i);

  Ret_M128_M128(__m256, _mm256_setr_m128);
  Ret_M128d_M128d(__m256d, _mm256_setr_m128d);
  Ret_M128i_M128i(__m256i, _mm256_setr_m128i);
}

void test_cast(void) {
  Ret_M256d(__m256, _mm256_castpd_ps);
  Ret_M256d(__m256i, _mm256_castpd_si256);
  Ret_M256(__m256d, _mm256_castps_pd);
  Ret_M256(__m256i, _mm256_castps_si256);
  Ret_M256i(__m256d, _mm256_castsi256_pd);
  Ret_M256i(__m256, _mm256_castsi256_ps);

  Ret_M256d(__m128d, _mm256_castpd256_pd128);
  Ret_M256(__m128, _mm256_castps256_ps128);
  Ret_M256i(__m128i, _mm256_castsi256_si128);
  Ret_M128d(__m256d, _mm256_castpd128_pd256);
  Ret_M128(__m256, _mm256_castps128_ps256);
  Ret_M128i(__m256i, _mm256_castsi128_si256);

  Ret_M128d(__m256d, _mm256_zextpd128_pd256);
  Ret_M128(__m256, _mm256_zextps128_ps256);
  Ret_M128i(__m256i, _mm256_zextsi128_si256);
}

int main() {
  assert(numInterestingFloats % 8 == 0);
  assert(numInterestingInts % 8 == 0);
  assert(numInterestingDoubles % 4 == 0);

  test_arithmetic();
  test_special_math();
  test_elementary_math();
  test_logical();
  test_swizzle();
  test_convert();
  test_move();
  test_compare();
  test_misc();
  test_load();
  test_store();
  test_undef();
  test_set();
  test_cast();
}
