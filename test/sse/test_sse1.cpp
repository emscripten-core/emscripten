/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses SSE1 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <xmmintrin.h>
#include "test_sse.h"

bool testNaNBits = true;

float* interesting_floats;
int numInterestingFloats;

uint32_t* interesting_ints;
int numInterestingInts;

char str[256] = {};

void mxcsr(void) {
  // MXCSR register state
  printf("%08X\n", _mm_getcsr());
  printf("%08X\n", _MM_GET_EXCEPTION_MASK());
  printf("%08X\n", _MM_GET_EXCEPTION_STATE());
  printf("%08X\n", _MM_GET_FLUSH_ZERO_MODE());
  printf("%08X\n", _MM_GET_ROUNDING_MODE());
}

void arith(void) {
  // SSE1 Arithmetic instructions:
  Ret_M128_M128(__m128, _mm_add_ps);
  Ret_M128_M128(__m128, _mm_add_ss);
  Ret_M128_M128(__m128, _mm_div_ps);
  Ret_M128_M128(__m128, _mm_div_ss);
  Ret_M128_M128(__m128, _mm_mul_ps);
  Ret_M128_M128(__m128, _mm_mul_ss);
  Ret_M128_M128(__m128, _mm_sub_ps);
  Ret_M128_M128(__m128, _mm_sub_ss);
}

void math(void) {
  // SSE1 Elementary Math functions:
  Ret_M128approx(__m128, _mm_rcp_ps);
  Ret_M128approx(__m128, _mm_rcp_ss);
  Ret_M128approx(__m128, _mm_rsqrt_ps);
  Ret_M128approx(__m128, _mm_rsqrt_ss);
  Ret_M128approx(__m128, _mm_sqrt_ps);
  Ret_M128approx(__m128, _mm_sqrt_ss);
}

void logic(void) {
  // SSE1 Logical instructions:
  Ret_M128_M128(__m128, _mm_and_ps);
  Ret_M128_M128(__m128, _mm_andnot_ps);
  Ret_M128_M128(__m128, _mm_or_ps);
  Ret_M128_M128(__m128, _mm_xor_ps);
}

void compare(void) {
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
}

void convert(void) {
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
}

void load(void) {
  // SSE1 Load functions:
  Ret_FloatPtr(__m128, _mm_load_ps, 4, 4);
  Ret_FloatPtr(__m128, _mm_load_ps1, 1, 1);
  Ret_FloatPtr(__m128, _mm_load_ss, 1, 1);
  Ret_FloatPtr(__m128, _mm_load1_ps, 1, 1);
  Ret_M128_FloatPtr(__m128, _mm_loadh_pi, __m64*, 2, 1);
  Ret_M128_FloatPtr(__m128, _mm_loadl_pi, __m64*, 2, 1);
  Ret_FloatPtr(__m128, _mm_loadr_ps, 4, 4);
  Ret_FloatPtr(__m128, _mm_loadu_ps, 4, 1);
}

void misc(void) {
  // SSE1 Miscellaneous functions:
  Ret_M128(int, _mm_movemask_ps);
}

void move(void) {
  // SSE1 Move functions:
  Ret_M128_M128(__m128, _mm_move_ss);
  Ret_M128_M128(__m128, _mm_movehl_ps);
  Ret_M128_M128(__m128, _mm_movelh_ps);
}

void set(void) {
  // SSE1 Set functions:
  Ret_Float4(__m128, _mm_set_ps, 1);
  Ret_Float(__m128, _mm_set_ps1, 1);
  Ret_Float(__m128, _mm_set_ss, 1);
  Ret_Float(__m128, _mm_set1_ps, 1);
  Ret_Float4(__m128, _mm_setr_ps, 1);

  __m128 zero = _mm_setzero_ps();
  tostr(&zero, str);
  printf("_mm_setzero_ps() = %s\n", str);
}

void special_math(void) {
  // SSE1 Special Math instructions:
  Ret_M128_M128(__m128, _mm_max_ps);
  Ret_M128_M128(__m128, _mm_max_ss);
  Ret_M128_M128(__m128, _mm_min_ps);
  Ret_M128_M128(__m128, _mm_min_ss);
}

void store(void) {
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
}

void swizzle(void) {
  // SSE1 Swizzle instructions:
  Ret_M128_M128_Tint(__m128, _mm_shuffle_ps);

  __m128 m1 = _mm_set_ps(1.f, 2.f, 3.f, 4.f);
  __m128 m2 = _mm_set_ps(5.f, 6.f, 7.f, 8.f);
  __m128 m3 = _mm_set_ps(9.f, 10.f, 11.f, 12.f);
  __m128 m4 = _mm_set_ps(13.f, 14.f, 15.f, 16.f);
  _MM_TRANSPOSE4_PS(m1, m2, m3, m4);
  tostr(&m1, str);
  printf("_MM_TRANSPOSE4_PS: m1 = %s\n", str);
  tostr(&m2, str);
  printf("_MM_TRANSPOSE4_PS: m2 = %s\n", str);
  tostr(&m3, str);
  printf("_MM_TRANSPOSE4_PS: m3 = %s\n", str);
  tostr(&m4, str);
  printf("_MM_TRANSPOSE4_PS: m4 = %s\n", str);

  Ret_M128_M128(__m128, _mm_unpackhi_ps);
  Ret_M128_M128(__m128, _mm_unpacklo_ps);
}

void alloc(void) {
  // _mm_malloc and _mm_free
  void* ptr = _mm_malloc(32, 16);
  assert(((uintptr_t)ptr & 0xF) == 0);
  _mm_free(ptr);
}

void undef(void) {
#ifdef __EMSCRIPTEN__
  _mm_undefined();
  _mm_undefined_ps();
#endif
}

int main() {
  interesting_floats = get_interesting_floats();
  numInterestingFloats =
    sizeof(interesting_floats_) / sizeof(interesting_floats_[0]);
  assert(numInterestingFloats % 4 == 0);

  interesting_ints = get_interesting_ints();
  numInterestingInts = sizeof(interesting_ints_) / sizeof(interesting_ints_[0]);
  assert(numInterestingInts % 4 == 0);

  mxcsr();
  arith();
  math();
  logic();
  compare();
  convert();
  load();
  misc();
  move();
  set();
  special_math();
  store();
  swizzle();
  alloc();
  undef();
}
