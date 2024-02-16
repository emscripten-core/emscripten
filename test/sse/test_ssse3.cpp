/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses SSSE3 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <tmmintrin.h>
#include "test_sse.h"

bool testNaNBits = true;

float *interesting_floats = get_interesting_floats();
int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
uint32_t *interesting_ints = get_interesting_ints();
int numInterestingInts = sizeof(interesting_ints_)/sizeof(interesting_ints_[0]);
double *interesting_doubles = get_interesting_doubles();
int numInterestingDoubles = sizeof(interesting_doubles_)/sizeof(interesting_doubles_[0]);

int main() {
  assert(numInterestingFloats % 4 == 0);
  assert(numInterestingInts % 4 == 0);
  assert(numInterestingDoubles % 4 == 0);

  Ret_M128i(__m128i, _mm_abs_epi8);
  Ret_M128i(__m128i, _mm_abs_epi16);
  Ret_M128i(__m128i, _mm_abs_epi32);
  Ret_M128i_M128i_Tint(__m128i, _mm_alignr_epi8);
  M128i_M128i_M128i(_mm_hadd_epi16);
  M128i_M128i_M128i(_mm_hadd_epi32);
  M128i_M128i_M128i(_mm_hadds_epi16);
  M128i_M128i_M128i(_mm_hsub_epi16);
  M128i_M128i_M128i(_mm_hsub_epi32);
  M128i_M128i_M128i(_mm_hsubs_epi16);
  M128i_M128i_M128i(_mm_maddubs_epi16);
  M128i_M128i_M128i(_mm_mulhrs_epi16);
  M128i_M128i_M128i(_mm_shuffle_epi8);
  M128i_M128i_M128i(_mm_sign_epi16);
  M128i_M128i_M128i(_mm_sign_epi32);
  M128i_M128i_M128i(_mm_sign_epi8);
}
