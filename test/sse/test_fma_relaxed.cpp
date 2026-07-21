/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// Exhaustive FMA test — compare against native x86 output.
// This test should be run with -mrelaxed-simd, and on an x86 host, so that
// Wasm relaxed SIMD FMA lowers to a hardware fused multiply-add matching
// native x86 FMA behavior bit-for-bit.

// immintrin.h must be included before test_sse.h
// clang-format off
#include <immintrin.h>
#include "test_sse.h"
// clang-format on

bool testNaNBits = false;

float* interesting_floats = get_interesting_floats();
int numInterestingFloats =
  sizeof(interesting_floats_) / sizeof(interesting_floats_[0]);
uint32_t* interesting_ints = get_interesting_ints();
int numInterestingInts =
  sizeof(interesting_ints_) / sizeof(interesting_ints_[0]);
double* interesting_doubles = get_interesting_doubles();
int numInterestingDoubles =
  sizeof(interesting_doubles_) / sizeof(interesting_doubles_[0]);

void test_fmadd(void) {
  Ret_M128_M128_M128(__m128, _mm_fmadd_ps);
  Ret_M128d_M128d_M128d(__m128d, _mm_fmadd_pd);
  Ret_M128_M128_M128(__m128, _mm_fmadd_ss);
  Ret_M128d_M128d_M128d(__m128d, _mm_fmadd_sd);
#ifdef __AVX__
  Ret_M256_M256_M256(__m256, _mm256_fmadd_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_fmadd_pd);
#endif
}

void test_fmsub(void) {
  Ret_M128_M128_M128(__m128, _mm_fmsub_ps);
  Ret_M128d_M128d_M128d(__m128d, _mm_fmsub_pd);
  Ret_M128_M128_M128(__m128, _mm_fmsub_ss);
  Ret_M128d_M128d_M128d(__m128d, _mm_fmsub_sd);
#ifdef __AVX__
  Ret_M256_M256_M256(__m256, _mm256_fmsub_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_fmsub_pd);
#endif
}

void test_fnmadd(void) {
  Ret_M128_M128_M128(__m128, _mm_fnmadd_ps);
  Ret_M128d_M128d_M128d(__m128d, _mm_fnmadd_pd);
  Ret_M128_M128_M128(__m128, _mm_fnmadd_ss);
  Ret_M128d_M128d_M128d(__m128d, _mm_fnmadd_sd);
#ifdef __AVX__
  Ret_M256_M256_M256(__m256, _mm256_fnmadd_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_fnmadd_pd);
#endif
}

void test_fnmsub(void) {
  Ret_M128_M128_M128(__m128, _mm_fnmsub_ps);
  Ret_M128d_M128d_M128d(__m128d, _mm_fnmsub_pd);
  Ret_M128_M128_M128(__m128, _mm_fnmsub_ss);
  Ret_M128d_M128d_M128d(__m128d, _mm_fnmsub_sd);
#ifdef __AVX__
  Ret_M256_M256_M256(__m256, _mm256_fnmsub_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_fnmsub_pd);
#endif
}

void test_fmaddsub(void) {
  Ret_M128_M128_M128(__m128, _mm_fmaddsub_ps);
  Ret_M128d_M128d_M128d(__m128d, _mm_fmaddsub_pd);
#ifdef __AVX__
  Ret_M256_M256_M256(__m256, _mm256_fmaddsub_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_fmaddsub_pd);
#endif
}

void test_fmsubadd(void) {
  Ret_M128_M128_M128(__m128, _mm_fmsubadd_ps);
  Ret_M128d_M128d_M128d(__m128d, _mm_fmsubadd_pd);
#ifdef __AVX__
  Ret_M256_M256_M256(__m256, _mm256_fmsubadd_ps);
  Ret_M256d_M256d_M256d(__m256d, _mm256_fmsubadd_pd);
#endif
}

int main() {
  assert(numInterestingFloats % 8 == 0);
  assert(numInterestingInts % 8 == 0);
  assert(numInterestingDoubles % 4 == 0);

  test_fmadd();
  test_fmsub();
  test_fnmadd();
  test_fnmsub();
  test_fmaddsub();
  test_fmsubadd();
}
