/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Self-contained FMA intrinsics test. Unlike other SSE tests, this does NOT
 * compare against native x86 output because FMA emulation (separate mul+add)
 * produces different results from true FMA (single rounding) for edge cases
 * near overflow/underflow boundaries. Instead, we test with well-behaved values
 * where emulated and true FMA agree exactly.
 */

#include <assert.h>
#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

static void check_f(const char* name, float got, float expected) {
  if (got == expected || (isnan(got) && isnan(expected))) {
    tests_passed++;
  } else {
    printf("FAIL %s: got %a, expected %a\n", name, got, expected);
    tests_failed++;
  }
}

static void check_d(const char* name, double got, double expected) {
  if (got == expected || (isnan(got) && isnan(expected))) {
    tests_passed++;
  } else {
    printf("FAIL %s: got %la, expected %la\n", name, got, expected);
    tests_failed++;
  }
}

/* Bit-exact comparison. Unlike check_f/check_d (which use ==), this
   distinguishes +0.0 from -0.0, so it can verify sign-of-zero results. */
static void check_f_bits(const char* name, float got, float expected) {
  if (memcmp(&got, &expected, sizeof(float)) == 0) {
    tests_passed++;
  } else {
    printf("FAIL %s: got %a, expected %a\n", name, got, expected);
    tests_failed++;
  }
}

static void check_d_bits(const char* name, double got, double expected) {
  if (memcmp(&got, &expected, sizeof(double)) == 0) {
    tests_passed++;
  } else {
    printf("FAIL %s: got %la, expected %la\n", name, got, expected);
    tests_failed++;
  }
}

/* Helper to extract float lanes */
static void storeu_ps(float* out, __m128 v) { _mm_storeu_ps(out, v); }
static void storeu_pd(double* out, __m128d v) { _mm_storeu_pd(out, v); }

/* ============================================================
 * 128-bit packed float tests
 * ============================================================ */
void test_128_ps(void) {
  __m128 a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
  __m128 b = _mm_set_ps(8.0f, 7.0f, 6.0f, 5.0f);
  __m128 c = _mm_set_ps(12.0f, 11.0f, 10.0f, 9.0f);
  float out[4];

  /* fmadd: a*b + c = {1*5+9, 2*6+10, 3*7+11, 4*8+12} = {14, 22, 32, 44} */
  storeu_ps(out, _mm_fmadd_ps(a, b, c));
  check_f("fmadd_ps[0]", out[0], 14.0f);
  check_f("fmadd_ps[1]", out[1], 22.0f);
  check_f("fmadd_ps[2]", out[2], 32.0f);
  check_f("fmadd_ps[3]", out[3], 44.0f);

  /* fmsub: a*b - c = {-4, 2, 10, 20} */
  storeu_ps(out, _mm_fmsub_ps(a, b, c));
  check_f("fmsub_ps[0]", out[0], -4.0f);
  check_f("fmsub_ps[1]", out[1], 2.0f);
  check_f("fmsub_ps[2]", out[2], 10.0f);
  check_f("fmsub_ps[3]", out[3], 20.0f);

  /* fnmadd: -(a*b) + c = {4, -2, -10, -20} */
  storeu_ps(out, _mm_fnmadd_ps(a, b, c));
  check_f("fnmadd_ps[0]", out[0], 4.0f);
  check_f("fnmadd_ps[1]", out[1], -2.0f);
  check_f("fnmadd_ps[2]", out[2], -10.0f);
  check_f("fnmadd_ps[3]", out[3], -20.0f);

  /* fnmsub: -(a*b) - c = {-14, -22, -32, -44} */
  storeu_ps(out, _mm_fnmsub_ps(a, b, c));
  check_f("fnmsub_ps[0]", out[0], -14.0f);
  check_f("fnmsub_ps[1]", out[1], -22.0f);
  check_f("fnmsub_ps[2]", out[2], -32.0f);
  check_f("fnmsub_ps[3]", out[3], -44.0f);

  /* fmaddsub: even=a*b-c, odd=a*b+c = {-4, 22, 10, 44} */
  storeu_ps(out, _mm_fmaddsub_ps(a, b, c));
  check_f("fmaddsub_ps[0]", out[0], -4.0f);
  check_f("fmaddsub_ps[1]", out[1], 22.0f);
  check_f("fmaddsub_ps[2]", out[2], 10.0f);
  check_f("fmaddsub_ps[3]", out[3], 44.0f);

  /* fmsubadd: even=a*b+c, odd=a*b-c = {14, 2, 32, 20} */
  storeu_ps(out, _mm_fmsubadd_ps(a, b, c));
  check_f("fmsubadd_ps[0]", out[0], 14.0f);
  check_f("fmsubadd_ps[1]", out[1], 2.0f);
  check_f("fmsubadd_ps[2]", out[2], 32.0f);
  check_f("fmsubadd_ps[3]", out[3], 20.0f);
}

/* ============================================================
 * 128-bit packed double tests
 * ============================================================ */
void test_128_pd(void) {
  __m128d a = _mm_set_pd(2.0, 1.0);
  __m128d b = _mm_set_pd(4.0, 3.0);
  __m128d c = _mm_set_pd(6.0, 5.0);
  double out[2];

  /* fmadd: {1*3+5, 2*4+6} = {8, 14} */
  storeu_pd(out, _mm_fmadd_pd(a, b, c));
  check_d("fmadd_pd[0]", out[0], 8.0);
  check_d("fmadd_pd[1]", out[1], 14.0);

  /* fmsub: {1*3-5, 2*4-6} = {-2, 2} */
  storeu_pd(out, _mm_fmsub_pd(a, b, c));
  check_d("fmsub_pd[0]", out[0], -2.0);
  check_d("fmsub_pd[1]", out[1], 2.0);

  /* fnmadd: {-(1*3)+5, -(2*4)+6} = {2, -2} */
  storeu_pd(out, _mm_fnmadd_pd(a, b, c));
  check_d("fnmadd_pd[0]", out[0], 2.0);
  check_d("fnmadd_pd[1]", out[1], -2.0);

  /* fnmsub: {-(1*3)-5, -(2*4)-6} = {-8, -14} */
  storeu_pd(out, _mm_fnmsub_pd(a, b, c));
  check_d("fnmsub_pd[0]", out[0], -8.0);
  check_d("fnmsub_pd[1]", out[1], -14.0);

  /* fmaddsub: {1*3-5, 2*4+6} = {-2, 14} */
  storeu_pd(out, _mm_fmaddsub_pd(a, b, c));
  check_d("fmaddsub_pd[0]", out[0], -2.0);
  check_d("fmaddsub_pd[1]", out[1], 14.0);

  /* fmsubadd: {1*3+5, 2*4-6} = {8, 2} */
  storeu_pd(out, _mm_fmsubadd_pd(a, b, c));
  check_d("fmsubadd_pd[0]", out[0], 8.0);
  check_d("fmsubadd_pd[1]", out[1], 2.0);
}

/* ============================================================
 * Scalar float tests — only element 0, upper from a
 * ============================================================ */
void test_scalar_ss(void) {
  __m128 a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
  __m128 b = _mm_set_ps(8.0f, 7.0f, 6.0f, 5.0f);
  __m128 c = _mm_set_ps(12.0f, 11.0f, 10.0f, 9.0f);
  float out[4];

  /* fmadd_ss: elem0 = 1*5+9 = 14, upper = {2, 3, 4} from a */
  storeu_ps(out, _mm_fmadd_ss(a, b, c));
  check_f("fmadd_ss[0]", out[0], 14.0f);
  check_f("fmadd_ss[1]", out[1], 2.0f);
  check_f("fmadd_ss[2]", out[2], 3.0f);
  check_f("fmadd_ss[3]", out[3], 4.0f);

  /* fmsub_ss: elem0 = 1*5-9 = -4 */
  storeu_ps(out, _mm_fmsub_ss(a, b, c));
  check_f("fmsub_ss[0]", out[0], -4.0f);
  check_f("fmsub_ss[1]", out[1], 2.0f);

  /* fnmadd_ss: elem0 = -(1*5)+9 = 4 */
  storeu_ps(out, _mm_fnmadd_ss(a, b, c));
  check_f("fnmadd_ss[0]", out[0], 4.0f);
  check_f("fnmadd_ss[1]", out[1], 2.0f);

  /* fnmsub_ss: elem0 = -(1*5)-9 = -14 */
  storeu_ps(out, _mm_fnmsub_ss(a, b, c));
  check_f("fnmsub_ss[0]", out[0], -14.0f);
  check_f("fnmsub_ss[1]", out[1], 2.0f);
}

/* ============================================================
 * Scalar double tests — only element 0, upper from a
 * ============================================================ */
void test_scalar_sd(void) {
  __m128d a = _mm_set_pd(2.0, 1.0);
  __m128d b = _mm_set_pd(4.0, 3.0);
  __m128d c = _mm_set_pd(6.0, 5.0);
  double out[2];

  /* fmadd_sd: elem0 = 1*3+5 = 8, elem1 = 2 from a */
  storeu_pd(out, _mm_fmadd_sd(a, b, c));
  check_d("fmadd_sd[0]", out[0], 8.0);
  check_d("fmadd_sd[1]", out[1], 2.0);

  /* fmsub_sd: elem0 = 1*3-5 = -2 */
  storeu_pd(out, _mm_fmsub_sd(a, b, c));
  check_d("fmsub_sd[0]", out[0], -2.0);
  check_d("fmsub_sd[1]", out[1], 2.0);

  /* fnmadd_sd: elem0 = -(1*3)+5 = 2 */
  storeu_pd(out, _mm_fnmadd_sd(a, b, c));
  check_d("fnmadd_sd[0]", out[0], 2.0);
  check_d("fnmadd_sd[1]", out[1], 2.0);

  /* fnmsub_sd: elem0 = -(1*3)-5 = -8 */
  storeu_pd(out, _mm_fnmsub_sd(a, b, c));
  check_d("fnmsub_sd[0]", out[0], -8.0);
  check_d("fnmsub_sd[1]", out[1], 2.0);
}

/* ============================================================
 * Special values: infinities, NaN, signed zero.
 *
 * Every expected value here is identical whether computed by true fused
 * FMA (single rounding) or by the emulated mul+add path (two roundings),
 * so this test is valid in both -mrelaxed-simd and -msimd128-only builds.
 *
 * To keep that property:
 *  - Infinities come from actual inf inputs (not from overflow), so the
 *    product is genuinely infinite in both paths.
 *  - inf - inf = NaN cases use an exact infinite product.
 *  - Signed-zero results are produced via multiply sign propagation
 *    (e.g. -0.0 * 1.0), never via product negation, which the emulated
 *    path implements as 0 - x and would round differently.
 * ============================================================ */
void test_special_ps(void) {
  const float inf = INFINITY;
  float out[4];

  /* fmadd: a*b + c
     lane0: inf*2 + 1        = +inf
     lane1: 2*3 + (-inf)     = -inf
     lane2: nan*1 + 5        = nan
     lane3: inf*1 + (-inf)   = nan  (inf - inf) */
  __m128 a = _mm_set_ps(inf, NAN, 2.0f, inf);
  __m128 b = _mm_set_ps(1.0f, 1.0f, 3.0f, 2.0f);
  __m128 c = _mm_set_ps(-inf, 5.0f, -inf, 1.0f);
  storeu_ps(out, _mm_fmadd_ps(a, b, c));
  check_f("special_fmadd_ps[0]", out[0], inf);
  check_f("special_fmadd_ps[1]", out[1], -inf);
  check_f("special_fmadd_ps[2]", out[2], NAN);
  check_f("special_fmadd_ps[3]", out[3], NAN);

  /* fmsub: a*b - c
     lane0: inf*1 - inf      = nan
     lane1: 2*3 - 1          = 5
     lane2: -0.0*1.0 - 0.0   = -0.0  (signed zero)
     lane3: inf*2 - 5        = +inf */
  a = _mm_set_ps(inf, -0.0f, 2.0f, inf);
  b = _mm_set_ps(2.0f, 1.0f, 3.0f, 1.0f);
  c = _mm_set_ps(5.0f, 0.0f, 1.0f, inf);
  storeu_ps(out, _mm_fmsub_ps(a, b, c));
  check_f("special_fmsub_ps[0]", out[0], NAN);
  check_f("special_fmsub_ps[1]", out[1], 5.0f);
  check_f_bits("special_fmsub_ps[2]", out[2], -0.0f);
  check_f("special_fmsub_ps[3]", out[3], inf);

  /* fnmadd: -(a*b) + c
     lane0: -(inf*1) + inf   = nan
     lane1: -(2*3) + 1       = -5
     lane2: -(inf*1) + 0     = -inf
     lane3: -(2*3) + inf     = +inf */
  a = _mm_set_ps(2.0f, inf, 2.0f, inf);
  b = _mm_set_ps(3.0f, 1.0f, 3.0f, 1.0f);
  c = _mm_set_ps(inf, 0.0f, 1.0f, inf);
  storeu_ps(out, _mm_fnmadd_ps(a, b, c));
  check_f("special_fnmadd_ps[0]", out[0], NAN);
  check_f("special_fnmadd_ps[1]", out[1], -5.0f);
  check_f("special_fnmadd_ps[2]", out[2], -inf);
  check_f("special_fnmadd_ps[3]", out[3], inf);

  /* fnmsub: -(a*b) - c
     lane0: -(2*3) - 1       = -7
     lane1: -(inf*1) - 1     = -inf
     lane2: -(inf*1) - inf   = -inf
     lane3: -(nan*1) - 1     = nan */
  a = _mm_set_ps(NAN, inf, inf, 2.0f);
  b = _mm_set_ps(1.0f, 1.0f, 1.0f, 3.0f);
  c = _mm_set_ps(1.0f, inf, 1.0f, 1.0f);
  storeu_ps(out, _mm_fnmsub_ps(a, b, c));
  check_f("special_fnmsub_ps[0]", out[0], -7.0f);
  check_f("special_fnmsub_ps[1]", out[1], -inf);
  check_f("special_fnmsub_ps[2]", out[2], -inf);
  check_f("special_fnmsub_ps[3]", out[3], NAN);
}

void test_special_pd(void) {
  const double inf = INFINITY;
  double out[2];

  /* fmadd: lane0: -0.0*1.0 + (-0.0) = -0.0 (signed zero)
            lane1: inf*1 + (-inf)    = nan */
  __m128d a = _mm_set_pd(inf, -0.0);
  __m128d b = _mm_set_pd(1.0, 1.0);
  __m128d c = _mm_set_pd(-inf, -0.0);
  storeu_pd(out, _mm_fmadd_pd(a, b, c));
  check_d_bits("special_fmadd_pd[0]", out[0], -0.0);
  check_d("special_fmadd_pd[1]", out[1], NAN);

  /* fnmadd: lane0: -(inf*1) + 0 = -inf
             lane1: -(2*3) + 1   = -5 */
  a = _mm_set_pd(2.0, inf);
  b = _mm_set_pd(3.0, 1.0);
  c = _mm_set_pd(1.0, 0.0);
  storeu_pd(out, _mm_fnmadd_pd(a, b, c));
  check_d("special_fnmadd_pd[0]", out[0], -inf);
  check_d("special_fnmadd_pd[1]", out[1], -5.0);
}

void test_special_scalar(void) {
  const float inf = INFINITY;
  float out[4];

  /* fmadd_ss: lane0 inf*1 + (-inf) = nan; upper lanes pass through from a */
  __m128 a = _mm_set_ps(4.0f, 3.0f, 2.0f, inf);
  __m128 b = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
  __m128 c = _mm_set_ps(0.0f, 0.0f, 0.0f, -inf);
  storeu_ps(out, _mm_fmadd_ss(a, b, c));
  check_f("special_fmadd_ss[0]", out[0], NAN);
  check_f("special_fmadd_ss[1]", out[1], 2.0f);
  check_f("special_fmadd_ss[2]", out[2], 3.0f);
  check_f("special_fmadd_ss[3]", out[3], 4.0f);

  const double dinf = INFINITY;
  double outd[2];

  /* fnmadd_sd: lane0 -(inf*1) + 0 = -inf; lane1 passes through from a */
  __m128d ad = _mm_set_pd(7.0, dinf);
  __m128d bd = _mm_set_pd(0.0, 1.0);
  __m128d cd = _mm_set_pd(0.0, 0.0);
  storeu_pd(outd, _mm_fnmadd_sd(ad, bd, cd));
  check_d("special_fnmadd_sd[0]", outd[0], -dinf);
  check_d("special_fnmadd_sd[1]", outd[1], 7.0);
}

/* ============================================================
 * 256-bit tests
 * ============================================================ */
#ifdef __AVX__
void test_256_ps(void) {
  __m256 a = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
  __m256 b =
    _mm256_set_ps(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f);
  __m256 c =
    _mm256_set_ps(24.0f, 23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f);
  float out[8];

  /* fmadd: a*b+c = {1*9+17, 2*10+18, 3*11+19, 4*12+20, 5*13+21, 6*14+22,
     7*15+23, 8*16+24} = {26, 38, 52, 68, 86, 106, 128, 152} */
  _mm256_storeu_ps(out, _mm256_fmadd_ps(a, b, c));
  check_f("256_fmadd_ps[0]", out[0], 26.0f);
  check_f("256_fmadd_ps[1]", out[1], 38.0f);
  check_f("256_fmadd_ps[2]", out[2], 52.0f);
  check_f("256_fmadd_ps[3]", out[3], 68.0f);
  check_f("256_fmadd_ps[4]", out[4], 86.0f);
  check_f("256_fmadd_ps[5]", out[5], 106.0f);
  check_f("256_fmadd_ps[6]", out[6], 128.0f);
  check_f("256_fmadd_ps[7]", out[7], 152.0f);

  /* fmsub: a*b-c */
  _mm256_storeu_ps(out, _mm256_fmsub_ps(a, b, c));
  check_f("256_fmsub_ps[0]", out[0], -8.0f);  /* 1*9-17 */
  check_f("256_fmsub_ps[7]", out[7], 104.0f); /* 8*16-24 */

  /* fnmadd: -(a*b)+c */
  _mm256_storeu_ps(out, _mm256_fnmadd_ps(a, b, c));
  check_f("256_fnmadd_ps[0]", out[0], 8.0f);    /* -(1*9)+17 */
  check_f("256_fnmadd_ps[7]", out[7], -104.0f); /* -(8*16)+24 */

  /* fnmsub: -(a*b)-c */
  _mm256_storeu_ps(out, _mm256_fnmsub_ps(a, b, c));
  check_f("256_fnmsub_ps[0]", out[0], -26.0f);  /* -(1*9)-17 */
  check_f("256_fnmsub_ps[7]", out[7], -152.0f); /* -(8*16)-24 */

  /* fmaddsub: even=a*b-c, odd=a*b+c */
  _mm256_storeu_ps(out, _mm256_fmaddsub_ps(a, b, c));
  check_f("256_fmaddsub_ps[0]", out[0], -8.0f); /* even: sub */
  check_f("256_fmaddsub_ps[1]", out[1], 38.0f); /* odd: add */
  check_f("256_fmaddsub_ps[2]", out[2], 14.0f); /* even: sub */
  check_f("256_fmaddsub_ps[3]", out[3], 68.0f); /* odd: add */

  /* fmsubadd: even=a*b+c, odd=a*b-c */
  _mm256_storeu_ps(out, _mm256_fmsubadd_ps(a, b, c));
  check_f("256_fmsubadd_ps[0]", out[0], 26.0f); /* even: add */
  check_f("256_fmsubadd_ps[1]", out[1], 2.0f);  /* odd: sub */
}

void test_256_pd(void) {
  __m256d a = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
  __m256d b = _mm256_set_pd(8.0, 7.0, 6.0, 5.0);
  __m256d c = _mm256_set_pd(12.0, 11.0, 10.0, 9.0);
  double out[4];

  /* fmadd: {1*5+9, 2*6+10, 3*7+11, 4*8+12} = {14, 22, 32, 44} */
  _mm256_storeu_pd(out, _mm256_fmadd_pd(a, b, c));
  check_d("256_fmadd_pd[0]", out[0], 14.0);
  check_d("256_fmadd_pd[1]", out[1], 22.0);
  check_d("256_fmadd_pd[2]", out[2], 32.0);
  check_d("256_fmadd_pd[3]", out[3], 44.0);

  /* fmsub */
  _mm256_storeu_pd(out, _mm256_fmsub_pd(a, b, c));
  check_d("256_fmsub_pd[0]", out[0], -4.0);
  check_d("256_fmsub_pd[3]", out[3], 20.0);

  /* fnmadd */
  _mm256_storeu_pd(out, _mm256_fnmadd_pd(a, b, c));
  check_d("256_fnmadd_pd[0]", out[0], 4.0);

  /* fnmsub */
  _mm256_storeu_pd(out, _mm256_fnmsub_pd(a, b, c));
  check_d("256_fnmsub_pd[0]", out[0], -14.0);

  /* fmaddsub: even=a*b-c, odd=a*b+c */
  _mm256_storeu_pd(out, _mm256_fmaddsub_pd(a, b, c));
  check_d("256_fmaddsub_pd[0]", out[0], -4.0); /* even: sub */
  check_d("256_fmaddsub_pd[1]", out[1], 22.0); /* odd: add */

  /* fmsubadd: even=a*b+c, odd=a*b-c */
  _mm256_storeu_pd(out, _mm256_fmsubadd_pd(a, b, c));
  check_d("256_fmsubadd_pd[0]", out[0], 14.0); /* even: add */
  check_d("256_fmsubadd_pd[1]", out[1], 2.0);  /* odd: sub */
}
#endif

/* ============================================================
 * Additional edge case tests with fractional values
 * ============================================================ */
void test_fractional(void) {
  /* Use values that exercise non-trivial math but stay in exact float range */
  __m128 a = _mm_set_ps(0.5f, -1.5f, 2.25f, -0.75f);
  __m128 b = _mm_set_ps(4.0f, -2.0f, 0.5f, 8.0f);
  __m128 c = _mm_set_ps(1.0f, 3.0f, -1.0f, 0.5f);
  float out[4];

  /* fmadd: {-0.75*8+0.5, 2.25*0.5-1, -1.5*-2+3, 0.5*4+1}
           = {-5.5, 0.125, 6, 3} */
  storeu_ps(out, _mm_fmadd_ps(a, b, c));
  check_f("frac_fmadd[0]", out[0], -5.5f);
  check_f("frac_fmadd[1]", out[1], 0.125f);
  check_f("frac_fmadd[2]", out[2], 6.0f);
  check_f("frac_fmadd[3]", out[3], 3.0f);

  __m128d ad = _mm_set_pd(-0.125, 3.5);
  __m128d bd = _mm_set_pd(16.0, 2.0);
  __m128d cd = _mm_set_pd(1.0, -3.0);
  double outd[2];

  /* fmadd: {3.5*2-3, -0.125*16+1} = {4, -1} */
  storeu_pd(outd, _mm_fmadd_pd(ad, bd, cd));
  check_d("frac_fmadd_pd[0]", outd[0], 4.0);
  check_d("frac_fmadd_pd[1]", outd[1], -1.0);
}

int main() {
  printf("Testing 128-bit packed float (ps)...\n");
  test_128_ps();

  printf("Testing 128-bit packed double (pd)...\n");
  test_128_pd();

  printf("Testing scalar float (ss)...\n");
  test_scalar_ss();

  printf("Testing scalar double (sd)...\n");
  test_scalar_sd();

#ifdef __AVX__
  printf("Testing 256-bit packed float (ps)...\n");
  test_256_ps();

  printf("Testing 256-bit packed double (pd)...\n");
  test_256_pd();
#endif

  printf("Testing fractional values...\n");
  test_fractional();

  printf("Testing special values (inf, nan, signed zero)...\n");
  test_special_ps();
  test_special_pd();
  test_special_scalar();

  printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
  if (tests_failed == 0) {
    printf("All FMA tests PASSED\n");
  }
  assert(tests_failed == 0);
  return 0;
}
