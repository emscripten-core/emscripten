/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <tmmintrin.h>
#include "benchmark_sse.h"

int main() {
  printf ("{ \"workload\": %u, \"results\": [\n", N);

  float *src_flt = alloc_float_buffer();
  float *src2_flt = alloc_float_buffer();
  float *dst_flt = alloc_float_buffer();
  for(int i = 0; i < N; ++i) src_flt[i] = (float)(1.0 + (double)rand() / RAND_MAX);
  for(int i = 0; i < N; ++i) src2_flt[i] = (float)(1.0 + (double)rand() / RAND_MAX);

  double *src_dbl = alloc_double_buffer();
  double *src2_dbl = alloc_double_buffer();
  double *dst_dbl = alloc_double_buffer();
  for(int i = 0; i < N; ++i) src_dbl[i] = 1.0 + (double)rand() / RAND_MAX;
  for(int i = 0; i < N; ++i) src2_dbl[i] = 1.0 + (double)rand() / RAND_MAX;

  int *src_int = alloc_int_buffer();
  int *src2_int = alloc_int_buffer();
  int *dst_int = alloc_int_buffer();
  for(int i = 0; i < N; ++i) src_int[i] = rand();
  for(int i = 0; i < N; ++i) src2_int[i] = rand();

  float scalarTime = 0.f;

  // Benchmarks start:
  SETCHART("abs");
  UNARYOP_I_I("_mm_abs_epi8", _mm_abs_epi8, _mm_load_si128((__m128i*)src_int));
  UNARYOP_I_I("_mm_abs_epi16", _mm_abs_epi16, _mm_load_si128((__m128i*)src_int));
  UNARYOP_I_I("_mm_abs_epi32", _mm_abs_epi32, _mm_load_si128((__m128i*)src_int));

  // TODO _mm_alignr_epi8

  SETCHART("horizontal add");
  BINARYOP_I_II("_mm_hadd_epi16", _mm_hadd_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_hadd_epi32", _mm_hadd_epi32, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_hadds_epi16", _mm_hadds_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));

  SETCHART("horizontal sub");
  BINARYOP_I_II("_mm_hsub_epi16", _mm_hsub_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_hsub_epi32", _mm_hsub_epi32, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_hsubs_epi16", _mm_hsubs_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));

  SETCHART("mul & shuffle");
  BINARYOP_I_II("_mm_maddubs_epi16", _mm_maddubs_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_mulhrs_epi16", _mm_mulhrs_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_shuffle_epi8", _mm_shuffle_epi8, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));

  SETCHART("sign");
  BINARYOP_I_II("_mm_sign_epi8", _mm_sign_epi8, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_sign_epi16", _mm_sign_epi16, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));
  BINARYOP_I_II("_mm_sign_epi32", _mm_sign_epi32, _mm_load_si128((__m128i*)src_int), _mm_load_si128((__m128i*)src2_int));

  // Benchmarks end:
  printf("]}\n");
}
