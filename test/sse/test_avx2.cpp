/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses AVX2 by calling different functions with different interesting
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
  Ret_M256i_M256i(__m256i, _mm256_add_epi8);
  Ret_M256i_M256i(__m256i, _mm256_add_epi16);
  Ret_M256i_M256i(__m256i, _mm256_add_epi32);
  Ret_M256i_M256i(__m256i, _mm256_add_epi64);

  Ret_M256i_M256i(__m256i, _mm256_adds_epi8);
  Ret_M256i_M256i(__m256i, _mm256_adds_epi16);
  Ret_M256i_M256i(__m256i, _mm256_adds_epu8);
  Ret_M256i_M256i(__m256i, _mm256_adds_epu16);

  Ret_M256i_M256i(__m256i, _mm256_hadd_epi16);
  Ret_M256i_M256i(__m256i, _mm256_hadd_epi32);
  Ret_M256i_M256i(__m256i, _mm256_hadds_epi16);

  Ret_M256i_M256i(__m256i, _mm256_hsub_epi16);
  Ret_M256i_M256i(__m256i, _mm256_hsub_epi32);
  Ret_M256i_M256i(__m256i, _mm256_hsubs_epi16);

  Ret_M256i_M256i(__m256i, _mm256_maddubs_epi16);
  Ret_M256i_M256i(__m256i, _mm256_madd_epi16);

  Ret_M256i_M256i(__m256i, _mm256_mul_epi32);
  Ret_M256i_M256i(__m256i, _mm256_mulhrs_epi16);
  Ret_M256i_M256i(__m256i, _mm256_mulhi_epu16);
  Ret_M256i_M256i(__m256i, _mm256_mulhi_epi16);
  Ret_M256i_M256i(__m256i, _mm256_mullo_epi16);
  Ret_M256i_M256i(__m256i, _mm256_mullo_epi32);
  Ret_M256i_M256i(__m256i, _mm256_mul_epu32);

  Ret_M256i_M256i(__m256i, _mm256_sad_epu8);

  Ret_M256i_M256i(__m256i, _mm256_sign_epi8);
  Ret_M256i_M256i(__m256i, _mm256_sign_epi16);
  Ret_M256i_M256i(__m256i, _mm256_sign_epi32);

  Ret_M256i_M256i(__m256i, _mm256_sub_epi8);
  Ret_M256i_M256i(__m256i, _mm256_sub_epi16);
  Ret_M256i_M256i(__m256i, _mm256_sub_epi32);
  Ret_M256i_M256i(__m256i, _mm256_sub_epi64);

  Ret_M256i_M256i(__m256i, _mm256_subs_epi8);
  Ret_M256i_M256i(__m256i, _mm256_subs_epi16);
  Ret_M256i_M256i(__m256i, _mm256_subs_epu8);
  Ret_M256i_M256i(__m256i, _mm256_subs_epu16);
}

void test_special_math(void) {
  Ret_M256i(__m256i, _mm256_abs_epi8);
  Ret_M256i(__m256i, _mm256_abs_epi16);
  Ret_M256i(__m256i, _mm256_abs_epi32);

  Ret_M256i_M256i(__m256i, _mm256_max_epi8);
  Ret_M256i_M256i(__m256i, _mm256_max_epi16);
  Ret_M256i_M256i(__m256i, _mm256_max_epi32);

  Ret_M256i_M256i(__m256i, _mm256_max_epu8);
  Ret_M256i_M256i(__m256i, _mm256_max_epu16);
  Ret_M256i_M256i(__m256i, _mm256_max_epu32);

  Ret_M256i_M256i(__m256i, _mm256_min_epi8);
  Ret_M256i_M256i(__m256i, _mm256_min_epi16);
  Ret_M256i_M256i(__m256i, _mm256_min_epi32);

  Ret_M256i_M256i(__m256i, _mm256_min_epu8);
  Ret_M256i_M256i(__m256i, _mm256_min_epu16);
  Ret_M256i_M256i(__m256i, _mm256_min_epu32);
}

void test_logical(void) {
  Ret_M256i_M256i(__m256i, _mm256_and_si256);
  Ret_M256i_M256i(__m256i, _mm256_andnot_si256);
  Ret_M256i_M256i(__m256i, _mm256_or_si256);
  Ret_M256i_M256i(__m256i, _mm256_xor_si256);
}

// Split test_swizzle to reduce memory consumption
void test_swizzle1(void) {
  Ret_M256i_M256i_M256i(__m256i, _mm256_blendv_epi8);
  Ret_M256i_M256i_Tint(__m256i, _mm256_blend_epi16);

  Ret_M256i_M256i(__m256i, _mm256_shuffle_epi8);
  Ret_M256i_Tint(__m256i, _mm256_shuffle_epi32);
  Ret_M256i_Tint(__m256i, _mm256_shufflehi_epi16);
  Ret_M256i_Tint(__m256i, _mm256_shufflelo_epi16);
}

void test_swizzle2(void) {
  Ret_M256i_M256i(__m256i, _mm256_unpackhi_epi8);
  Ret_M256i_M256i(__m256i, _mm256_unpackhi_epi16);
  Ret_M256i_M256i(__m256i, _mm256_unpackhi_epi32);
  Ret_M256i_M256i(__m256i, _mm256_unpackhi_epi64);

  Ret_M256i_M256i(__m256i, _mm256_unpacklo_epi8);
  Ret_M256i_M256i(__m256i, _mm256_unpacklo_epi16);
  Ret_M256i_M256i(__m256i, _mm256_unpacklo_epi32);
  Ret_M256i_M256i(__m256i, _mm256_unpacklo_epi64);

  Ret_M128(__m128, _mm_broadcastss_ps);
  Ret_M128d(__m128d, _mm_broadcastsd_pd);
  Ret_M128(__m256, _mm256_broadcastss_ps);
  Ret_M128d(__m256d, _mm256_broadcastsd_pd);
  Ret_M128i(__m256i, _mm256_broadcastsi128_si256);
  Ret_M128i(__m256i, _mm_broadcastsi128_si256);

  Ret_M128i_M128i_Tint(__m128i, _mm_blend_epi32);
  Ret_M256i_M256i_Tint(__m256i, _mm256_blend_epi32);

  Ret_M128i(__m128i, _mm_broadcastb_epi8);
  Ret_M128i(__m128i, _mm_broadcastw_epi16);
  Ret_M128i(__m128i, _mm_broadcastd_epi32);
  Ret_M128i(__m128i, _mm_broadcastq_epi64);
  Ret_M128i(__m256i, _mm256_broadcastb_epi8);
  Ret_M128i(__m256i, _mm256_broadcastw_epi16);
  Ret_M128i(__m256i, _mm256_broadcastd_epi32);
  Ret_M128i(__m256i, _mm256_broadcastq_epi64);

  Ret_M256i_M256i(__m256i, _mm256_permutevar8x32_epi32);
  Ret_M256_M256i(__m256, _mm256_permutevar8x32_ps);
  Ret_M256i_Tint(__m256i, _mm256_permute4x64_epi64);
}

void test_swizzle3(void) {
  Ret_M256d_Tint(__m256d, _mm256_permute4x64_pd);
  Ret_M256i_M256i_Tint(__m256i, _mm256_permute2x128_si256);

  Ret_M256i_Tint(__m128i, _mm256_extracti128_si256);
  Ret_M256i_M128i_Tint(__m256i, _mm256_inserti128_si256);
}

void test_swizzle(void) {
  test_swizzle1();
  test_swizzle2();
  test_swizzle3();
}

void test_convert(void) {
  Ret_M128i(__m256i, _mm256_cvtepi8_epi16);
  Ret_M128i(__m256i, _mm256_cvtepi8_epi32);
  Ret_M128i(__m256i, _mm256_cvtepi8_epi64);
  Ret_M128i(__m256i, _mm256_cvtepi16_epi32);
  Ret_M128i(__m256i, _mm256_cvtepi16_epi64);
  Ret_M128i(__m256i, _mm256_cvtepi32_epi64);

  Ret_M128i(__m256i, _mm256_cvtepu8_epi16);
  Ret_M128i(__m256i, _mm256_cvtepu8_epi32);
  Ret_M128i(__m256i, _mm256_cvtepu8_epi64);
  Ret_M128i(__m256i, _mm256_cvtepu16_epi32);
  Ret_M128i(__m256i, _mm256_cvtepu16_epi64);
  Ret_M128i(__m256i, _mm256_cvtepu32_epi64);
}

void test_compare(void) {
  Ret_M256i_M256i(__m256i, _mm256_cmpeq_epi8);
  Ret_M256i_M256i(__m256i, _mm256_cmpeq_epi16);
  Ret_M256i_M256i(__m256i, _mm256_cmpeq_epi32);
  Ret_M256i_M256i(__m256i, _mm256_cmpeq_epi64);

  Ret_M256i_M256i(__m256i, _mm256_cmpgt_epi8);
  Ret_M256i_M256i(__m256i, _mm256_cmpgt_epi16);
  Ret_M256i_M256i(__m256i, _mm256_cmpgt_epi32);
  Ret_M256i_M256i(__m256i, _mm256_cmpgt_epi64);
}

void test_misc(void) {
  Ret_M256i_M256i_Tint(__m256i, _mm256_mpsadbw_epu8);

  Ret_M256i_M256i(__m256i, _mm256_packs_epi16);
  Ret_M256i_M256i(__m256i, _mm256_packs_epi32);
  Ret_M256i_M256i(__m256i, _mm256_packus_epi16);
  Ret_M256i_M256i(__m256i, _mm256_packus_epi32);

  Ret_M256i_M256i_Tint(__m256i, _mm256_alignr_epi8);

  Ret_M256i(int, _mm256_movemask_epi8);
}

// Split test_load to reduce memory consumption
void test_load1(void) {
  Ret_IntPtr(__m256i, _mm256_stream_load_si256, __m256i*, 8, 8);

  Ret_IntPtr_M128i(__m128i, _mm_maskload_epi32, int32_t*, 4, 4);
  Ret_IntPtr_M256i(__m256i, _mm256_maskload_epi32, int32_t*, 8, 8);
  Ret_IntPtr_M128i(__m128i, _mm_maskload_epi64, long long const*, 4, 4);
  Ret_IntPtr_M256i(__m256i, _mm256_maskload_epi64, long long const*, 8, 8);

  Ret_M128d_DoublePtr_I32x4_M128d_Tint_body(__m128d, _mm_mask_i32gather_pd, 8);
  Ret_M256d_DoublePtr_I32x4_M256d_Tint_body(
    __m256d, _mm256_mask_i32gather_pd, 8);
  Ret_M128d_DoublePtr_I64x2_M128d_Tint_body(__m128d, _mm_mask_i64gather_pd, 8);
  Ret_M256d_DoublePtr_I64x4_M256d_Tint_body(
    __m256d, _mm256_mask_i64gather_pd, 8);

  Ret_M128_FloatPtr_I32x4_M128_Tint_body(__m128, _mm_mask_i32gather_ps, 4);
  Ret_M256_FloatPtr_I32x8_M256_Tint_body(__m256, _mm256_mask_i32gather_ps, 4);
  Ret_M128_FloatPtr_I64x2_M128_Tint_body(__m128, _mm_mask_i64gather_ps, 4);
  Ret_M128_FloatPtr_I64x4_M128_Tint_body(__m128, _mm256_mask_i64gather_ps, 4);

  Ret_M128i_Int32Ptr_I32x4_M128i_Tint_body(
    __m128i, _mm_mask_i32gather_epi32, 4);
  Ret_M256i_Int32Ptr_I32x8_M256i_Tint_body(
    __m256i, _mm256_mask_i32gather_epi32, 4);
  Ret_M128i_Int32Ptr_I64x2_M128i_Tint_body(
    __m128i, _mm_mask_i64gather_epi32, 4);
  Ret_M128i_Int32Ptr_I64x4_M128i_Tint_body(
    __m128i, _mm256_mask_i64gather_epi32, 4);

  Ret_M128i_Int64Ptr_I32x4_M128i_Tint_body(
    __m128i, _mm_mask_i32gather_epi64, 8);
  Ret_M256i_Int64Ptr_I32x4_M256i_Tint_body(
    __m256i, _mm256_mask_i32gather_epi64, 8);
  Ret_M128i_Int64Ptr_I64x2_M128i_Tint_body(
    __m128i, _mm_mask_i64gather_epi64, 8);
  Ret_M256i_Int64Ptr_I64x4_M256i_Tint_body(
    __m256i, _mm256_mask_i64gather_epi64, 8);
}

void test_load2(void) {
  Ret_DoublePtr_I32x4_Tint_body(__m128d, _mm_i32gather_pd, 8);
  Ret_DoublePtr_I32x4_Tint_body(__m256d, _mm256_i32gather_pd, 8);
  Ret_DoublePtr_I64x2_Tint_body(__m128d, _mm_i64gather_pd, 8);
  Ret_DoublePtr_I64x4_Tint_body(__m256d, _mm256_i64gather_pd, 8);

  Ret_FloatPtr_I32x4_Tint_body(__m128, _mm_i32gather_ps, 4);
  Ret_FloatPtr_I32x8_Tint_body(__m256, _mm256_i32gather_ps, 4);
  Ret_FloatPtr_I64x2_Tint_body(__m128, _mm_i64gather_ps, 4);
  Ret_FloatPtr_I64x4_Tint_body(__m128, _mm256_i64gather_ps, 4);

  Ret_IntPtr_I32x4_Tint_body(__m128i, _mm_i32gather_epi32, 4);
  Ret_IntPtr_I32x8_Tint_body(__m256i, _mm256_i32gather_epi32, 4);
  Ret_IntPtr_I64x2_Tint_body(__m128i, _mm_i64gather_epi32, 4);
  Ret_IntPtr_I64x4_Tint_body(__m128i, _mm256_i64gather_epi32, 4);

  Ret_Int64Ptr_I32x4_Tint_body(__m128i, _mm_i32gather_epi64, 8);
  Ret_Int64Ptr_I32x4_Tint_body(__m256i, _mm256_i32gather_epi64, 8);
  Ret_Int64Ptr_I64x2_Tint_body(__m128i, _mm_i64gather_epi64, 8);
  Ret_Int64Ptr_I64x4_Tint_body(__m256i, _mm256_i64gather_epi64, 8);
}

void test_load(void) {
  test_load1();
  test_load2();
}

void test_store(void) {
  void_OutIntPtr_M128i_M128i(_mm_maskstore_epi32, int*, 16, 4);
  void_OutIntPtr_M256i_M256i(_mm256_maskstore_epi32, int*, 32, 4);
  void_OutIntPtr_M128i_M128i(_mm_maskstore_epi64, long long*, 16, 8);
  void_OutIntPtr_M256i_M256i(_mm256_maskstore_epi64, long long*, 16, 8);
}

void test_statisticsa(void) {
  Ret_M256i_M256i(__m256i, _mm256_avg_epu16);
  Ret_M256i_M256i(__m256i, _mm256_avg_epu8);
}

// Split test_shift to reduce memory consumption
void test_shift1(void) {
  Ret_M256i_Tint(__m256i, _mm256_slli_si256);
  Ret_M256i_Tint(__m256i, _mm256_bslli_epi128);

  Ret_M256i_Tint(__m256i, _mm256_slli_epi16);
  Ret_M256i_Tint(__m256i, _mm256_slli_epi32);
}

void test_shift2(void) {
  Ret_M256i_Tint(__m256i, _mm256_slli_epi64);
  Ret_M256i_M128i(__m256i, _mm256_sll_epi16);
  Ret_M256i_M128i(__m256i, _mm256_sll_epi32);
  Ret_M256i_M128i(__m256i, _mm256_sll_epi64);

  Ret_M256i_Tint(__m256i, _mm256_srai_epi16);
  Ret_M256i_Tint(__m256i, _mm256_srai_epi32);
  Ret_M256i_M128i(__m256i, _mm256_sra_epi16);
  Ret_M256i_M128i(__m256i, _mm256_sra_epi32);

  Ret_M256i_Tint(__m256i, _mm256_srli_si256);
}

void test_shift3(void) {
  Ret_M256i_Tint(__m256i, _mm256_bsrli_epi128);

  Ret_M256i_Tint(__m256i, _mm256_srli_epi16);
  Ret_M256i_Tint(__m256i, _mm256_srli_epi32);
  Ret_M256i_Tint(__m256i, _mm256_srli_epi64);
  Ret_M256i_M128i(__m256i, _mm256_srl_epi16);
  Ret_M256i_M128i(__m256i, _mm256_srl_epi32);
  Ret_M256i_M128i(__m256i, _mm256_srl_epi64);

  Ret_M128i_M128i(__m128i, _mm_sllv_epi32);
  Ret_M256i_M256i(__m256i, _mm256_sllv_epi32);
  Ret_M128i_M128i(__m128i, _mm_sllv_epi64);
  Ret_M256i_M256i(__m256i, _mm256_sllv_epi64);

  Ret_M128i_M128i(__m128i, _mm_srav_epi32);
  Ret_M256i_M256i(__m256i, _mm256_srav_epi32);

  Ret_M128i_M128i(__m128i, _mm_srlv_epi32);
  Ret_M256i_M256i(__m256i, _mm256_srlv_epi32);
  Ret_M128i_M128i(__m128i, _mm_srlv_epi64);
  Ret_M256i_M256i(__m256i, _mm256_srlv_epi64);
}

void test_shift(void) {
  test_shift1();
  test_shift2();
  test_shift3();
}

int main() {
  assert(numInterestingFloats % 8 == 0);
  assert(numInterestingInts % 8 == 0);
  assert(numInterestingDoubles % 4 == 0);

  test_arithmetic();
  test_special_math();
  test_logical();
  test_swizzle();
  test_convert();
  test_compare();
  test_misc();
  test_load();
  test_store();
  test_statisticsa();
  test_shift();
}
