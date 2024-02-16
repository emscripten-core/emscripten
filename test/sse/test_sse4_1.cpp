/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses SSE4.1 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <smmintrin.h>
#include "test_sse.h"

bool testNaNBits = true;

float *interesting_floats = get_interesting_floats();
int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
uint32_t *interesting_ints = get_interesting_ints();
int numInterestingInts = sizeof(interesting_ints_)/sizeof(interesting_ints_[0]);
double *interesting_doubles = get_interesting_doubles();
int numInterestingDoubles = sizeof(interesting_doubles_)/sizeof(interesting_doubles_[0]);

void test_round() {
  Ret_M128d(__m128d, _mm_ceil_pd);
  Ret_M128(__m128, _mm_ceil_ps);
  Ret_M128d_M128d(__m128d, _mm_ceil_sd);
  Ret_M128_M128(__m128, _mm_ceil_ss);
  Ret_M128d(__m128d, _mm_floor_pd);
  Ret_M128(__m128, _mm_floor_ps);
  Ret_M128d_M128d(__m128d, _mm_floor_sd);
  Ret_M128_M128(__m128, _mm_floor_ss);
  Ret_M128d_Tint(__m128d, _mm_round_pd);
  Ret_M128_Tint(__m128, _mm_round_ps);
  Ret_M128d_M128d_Tint(__m128d, _mm_round_sd);
  Ret_M128_M128_Tint(__m128, _mm_round_ss);
}

int main() {
  assert(numInterestingFloats % 4 == 0);
  assert(numInterestingInts % 4 == 0);
  assert(numInterestingDoubles % 4 == 0);

  test_round();

  Ret_M128i_M128i_Tint(__m128i, _mm_blend_epi16);
  Ret_M128d_M128d_Tint(__m128d, _mm_blend_pd);
  Ret_M128_M128_Tint(__m128, _mm_blend_ps);
  Ret_M128i_M128i_M128i(__m128i, _mm_blendv_epi8);
  Ret_M128d_M128d_M128d(__m128d, _mm_blendv_pd);
  Ret_M128_M128_M128(__m128, _mm_blendv_ps);
  Ret_M128i_M128i(__m128i, _mm_cmpeq_epi64);
  Ret_M128i(__m128i, _mm_cvtepi16_epi32);
  Ret_M128i(__m128i, _mm_cvtepi16_epi64);
  Ret_M128i(__m128i, _mm_cvtepi32_epi64);
  Ret_M128i(__m128i, _mm_cvtepi8_epi16);
  Ret_M128i(__m128i, _mm_cvtepi8_epi32);
  Ret_M128i(__m128i, _mm_cvtepi8_epi64);
  Ret_M128i(__m128i, _mm_cvtepu16_epi32);
  Ret_M128i(__m128i, _mm_cvtepu16_epi64);
  Ret_M128i(__m128i, _mm_cvtepu32_epi64);
  Ret_M128i(__m128i, _mm_cvtepu8_epi16);
  Ret_M128i(__m128i, _mm_cvtepu8_epi32);
  Ret_M128i(__m128i, _mm_cvtepu8_epi64);
  testNaNBits = false;
  Ret_M128d_M128d_Tint(__m128d, _mm_dp_pd);
  Ret_M128_M128_Tint(__m128, _mm_dp_ps); // _mm_dp_ps emulation does not match NaN bit selection rules (seems to be unspecified)
  testNaNBits = true;
  Ret_M128i_Tint(int, _mm_extract_epi32);
  Ret_M128i_Tint(int64_t, _mm_extract_epi64);
  Ret_M128i_Tint(int, _mm_extract_epi8);
  Ret_M128_Tint(float, _mm_extract_ps);
  Ret_M128i_int_Tint(__m128i, _mm_insert_epi32);
  Ret_M128i_int_Tint(__m128i, _mm_insert_epi64);
  Ret_M128_M128_Tint(__m128, _mm_insert_ps);
  Ret_M128i_M128i(__m128i, _mm_max_epi32);
  Ret_M128i_M128i(__m128i, _mm_max_epi8);
  Ret_M128i_M128i(__m128i, _mm_max_epu16);
  Ret_M128i_M128i(__m128i, _mm_max_epu32);
  Ret_M128i_M128i(__m128i, _mm_min_epi32);
  Ret_M128i_M128i(__m128i, _mm_min_epi8);
  Ret_M128i_M128i(__m128i, _mm_min_epu16);
  Ret_M128i_M128i(__m128i, _mm_min_epu32);
  Ret_M128i(__m128i, _mm_minpos_epu16);
  Ret_M128i_M128i_Tint(__m128i, _mm_mpsadbw_epu8);
  Ret_M128i_M128i(__m128i, _mm_mul_epi32);
  Ret_M128i_M128i(__m128i, _mm_mullo_epi32);
  Ret_M128i_M128i(__m128i, _mm_packus_epi32);
  Ret_IntPtr(__m128i, _mm_stream_load_si128, __m128i*, 4, 4);
  Ret_M128i(int, _mm_test_all_ones);
  printf("_mm_test_all_ones(0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_all_ones(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_test_all_ones(0xFFFFFFFFFFFFFFFEull): %d\n", _mm_test_all_ones(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFEull)));
  printf("_mm_test_all_ones(0): %d\n", _mm_test_all_ones(_mm_set1_epi64x(0)));
  Ret_M128i_M128i(int, _mm_test_all_zeros);
  printf("_mm_test_all_zeros(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_all_zeros(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_test_all_zeros(0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_all_zeros(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFEull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_test_all_zeros(0, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_all_zeros(_mm_set1_epi64x(0), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  Ret_M128i_M128i(int, _mm_test_mix_ones_zeros);
  printf("_mm_test_mix_ones_zeros(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_mix_ones_zeros(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_test_mix_ones_zeros(0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_mix_ones_zeros(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFEull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_test_mix_ones_zeros(0, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_test_mix_ones_zeros(_mm_set1_epi64x(0), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  Ret_M128i_M128i(int, _mm_testc_si128);
  printf("_mm_testc_si128(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testc_si128(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_testc_si128(0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testc_si128(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFEull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_testc_si128(0, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testc_si128(_mm_set1_epi64x(0), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  Ret_M128i_M128i(int, _mm_testnzc_si128);
  printf("_mm_testnzc_si128(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testnzc_si128(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_testnzc_si128(0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testnzc_si128(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFEull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_testnzc_si128(0, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testnzc_si128(_mm_set1_epi64x(0), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  Ret_M128i_M128i(int, _mm_testz_si128);
  printf("_mm_testz_si128(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testz_si128(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_testz_si128(0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testz_si128(_mm_set1_epi64x(0xFFFFFFFFFFFFFFFEull), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
  printf("_mm_testz_si128(0, 0xFFFFFFFFFFFFFFFFull): %d\n", _mm_testz_si128(_mm_set1_epi64x(0), _mm_set1_epi64x(0xFFFFFFFFFFFFFFFFull)));
}
