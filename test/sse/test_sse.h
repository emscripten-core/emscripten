/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <float.h>
#include <assert.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define align1_int emscripten_align1_int
#define align1_int64 emscripten_align1_int64
#define align1_float emscripten_align1_float
#define align1_double emscripten_align1_double
#else
#define align1_int64 int64_t
#define align1_int int
#define align1_float float
#define align1_double double
#endif

// Recasts floating point representation of f to an integer.
uint32_t fcastu(float f) { return *(uint32_t*)&f; }
uint64_t dcastu(double f) { return *(uint64_t*)&f; }
float ucastf(uint32_t t) { return *(float*)&t; }
double ucastd(uint64_t t) { return *(double*)&t; }

// Data used in test. Store them global and access via a getter to confuse optimizer to not "solve" the whole test suite at compile-time,
// so that the operation will actually be performed at runtime, and not at compile-time. (Testing the capacity of the compiler to perform
// SIMD ops at compile-time would be interesting as well, but that's for another test)
__attribute__((aligned(32)))
float interesting_floats_[] = {
  -INFINITY,
  -FLT_MAX,
  -2.5f,
  -1.5f,
  -1.4f,
  -1.0f,
  -0.5f,
  -0.2f,
  -FLT_MIN,
  -0.f,
  0.f,
  1.401298464e-45f,
  FLT_MIN,
  0.3f,
  0.5f,
  0.8f,
  1.0f,
  1.5f,
  2.5f,
  3.5f,
  3.6f,
  FLT_MAX,
  INFINITY,
  NAN,
  ucastf(0x01020304),
  ucastf(0x80000000),
  ucastf(0x7FFFFFFF),
  ucastf(0xFFFFFFFF),
  -2.70497e+38f,
  -3.2995e-21f,
  3.40282e+38f,
  3.38211e+19f};

__attribute__((aligned(32)))
double interesting_doubles_[] = {
  -INFINITY,
  -FLT_MAX,
  -2.5,
  -1.5,
  -1.4,
  -1.0,
  -0.5,
  -0.2,
  -FLT_MIN,
  -0.0,
  0.0,
  1.401298464e-45,
  FLT_MIN,
  0.3,
  0.5,
  0.8,
  1.0,
  1.5,
  2.5,
  3.5,
  3.6,
  FLT_MAX,
  INFINITY,
  NAN,
  ucastd(0x0102030405060708ULL),
  ucastd(0x8000000000000000ULL),
  ucastd(0x7FFFFFFFFFFFFFFFULL),
  ucastd(0xFFFFFFFFFFFFFFFFULL)};

__attribute__((aligned(32)))
uint32_t interesting_ints_[] = {
  0,
  1,
  2,
  3,
  0x01020304,
  0x10203040,
  0x7FFFFFFF,
  0xFFFFFFFF,
  0xFFFFFFFE,
  0x12345678,
  0x9ABCDEF1,
  0x80000000,
  0x80808080,
  0x7F7F7F7F,
  0x01010101,
  0x11111111,
  0x20202020,
  0x0F0F0F0F,
  0xF0F0F0F0,
  fcastu(-INFINITY),
  fcastu(-FLT_MAX),
  fcastu(-2.5f),
  fcastu(-1.5f),
  fcastu(-1.4f),
  fcastu(-1.0f),
  fcastu(-0.5f),
  fcastu(-0.2f),
  fcastu(-FLT_MIN),
  0xF9301AB9,
  0x0039AB12,
  0x19302BCD,
  fcastu(1.401298464e-45f),
  fcastu(FLT_MIN),
  fcastu(0.3f),
  fcastu(0.5f),
  fcastu(0.8f),
  fcastu(1.0f),
  fcastu(1.5f),
  fcastu(2.5f),
  fcastu(3.5f),
  fcastu(3.6f),
  fcastu(FLT_MAX),
  fcastu(INFINITY),
  fcastu(NAN),
  0x000003FF,
  0xDDDDDDDD,
  0x88888888,
  0xEEEEEEEE};

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

bool IsNan(float f) { return (fcastu(f) << 1) > 0xFF000000u; }

// Replaces all occurrences of 'src' in string 'str' with 'dst', operating in place. strlen(dst) <= strlen(src).
void contract_inplace(char *str, const char *src, const char *dst)
{
  int dstLen = strlen(dst);
  int srcLen = strlen(src);
  int diff = srcLen - dstLen;
  assert(diff >= 0);

  while(true)
  {
    char *pos = strstr(str, src);
    if (!pos) return;
    str = pos;
    strcpy(pos, dst);
    pos += dstLen;
    strcpy(pos, pos + diff);
  }
}

// sprintf standard does not allow controlling how many leading zeros to use
// for printing out the exponent, and different compilers give different
// values. Perform a canonicalization step that enforces the printouts are
// the same.
void CanonicalizeStringComparisons(char *s)
{
  contract_inplace(s, "e+00", "e+");
  contract_inplace(s, "e-00", "e-");
  contract_inplace(s, "e+0", "e+");
  contract_inplace(s, "e-0", "e-");
  contract_inplace(s, "1.#INF", "inf");
}

// Global test state that is used per-test to determine whether to validate the state of exact NaN bits
// in specific functions.
extern bool testNaNBits;

char *SerializeFloat(float f, char *dstStr, bool approximate = false) {
  if (IsNan(f)) {
    uint32_t u = fcastu(f);
    int numChars = testNaNBits ? sprintf(dstStr, "NaN(0x%8X)", (unsigned int)u) : sprintf(dstStr, "NaN");
    return dstStr + numChars;
  } else {
    if (approximate > 0) {
      if (fabs(f) < FLT_MIN) // Flush denormals to zero (for _mm_rcp_ps)
        sprintf(dstStr, "%f", copysign(0.f, f));
      else if (fabs(f) >= 2.6e22f) // Flush large numbers to infinity (for _mm_rsqrt_ps)
        sprintf(dstStr, "%f", copysign(INFINITY, f));
      else
        sprintf(dstStr, "%.2g", f);
    } else {
      sprintf(dstStr, "%.9g", f);
    }
    CanonicalizeStringComparisons(dstStr);
    return dstStr + strlen(dstStr);
  }
}

char *SerializeDouble(double f, char *dstStr) {
  if (IsNan(f)) {
    uint64_t u = dcastu(f);
    int numChars = testNaNBits ? sprintf(dstStr, "NaN(0x%08X%08X)", (unsigned int)(u>>32), (unsigned int)u) : sprintf(dstStr, "NaN");
    return dstStr + numChars;
  } else {
    sprintf(dstStr, "%.17g", f);
    CanonicalizeStringComparisons(dstStr);
    return dstStr + strlen(dstStr);
  }
}

void tostr(__m128 *m, char *outstr) {
  union { __m128 m; float val[4]; } u;
  u.m = *m;
  char s[4][32];
  SerializeFloat(u.val[0], s[0]);
  SerializeFloat(u.val[1], s[1]);
  SerializeFloat(u.val[2], s[2]);
  SerializeFloat(u.val[3], s[3]);
  sprintf(outstr, "[%s,%s,%s,%s]", s[3], s[2], s[1], s[0]);
}

void tostr_approx(__m128 *m, char *outstr, bool approximate) {
  union { __m128 m; float val[4]; } u;
  u.m = *m;
  char s[4][32];
  SerializeFloat(u.val[0], s[0], approximate);
  SerializeFloat(u.val[1], s[1], approximate);
  SerializeFloat(u.val[2], s[2], approximate);
  SerializeFloat(u.val[3], s[3], approximate);
  sprintf(outstr, "[%s,%s,%s,%s]", s[3], s[2], s[1], s[0]);
}

void tostr(__m128i *m, char *outstr) {
  union { __m128i m; uint32_t val[4]; } u;
  u.m = *m;
  sprintf(outstr, "[0x%08X,0x%08X,0x%08X,0x%08X]", u.val[3], u.val[2], u.val[1], u.val[0]);
}

#ifdef __SSE2__

void tostr(__m128d *m, char *outstr) {
  union { __m128d m; double val[2]; } u;
  u.m = *m;
  char s[2][64];
  SerializeDouble(u.val[0], s[0]);
  SerializeDouble(u.val[1], s[1]);
  sprintf(outstr, "[%s,%s]", s[1], s[0]);
}

__m128i ExtractInRandomOrder(uint32_t *arr, int i, int n, int prime) {
  return _mm_set_epi32(arr[(i*prime)%n], arr[((i+1)*prime)%n], arr[((i+2)*prime)%n], arr[((i+3)*prime)%n]);
}

__m128d ExtractInRandomOrder(double *arr, int i, int n, int prime) {
  return _mm_set_pd(arr[(i*prime)%n], arr[((i+1)*prime)%n]);
}
#endif

void tostr(align1_int *m, char *outstr) {
  sprintf(outstr, "0x%08X", *m);
}

void tostr(align1_int64 *m, char *outstr) {
  sprintf(outstr, "0x%08X%08X", (int)(*m >> 32), (int)*m);
}

void tostr(align1_float *m, char *outstr) {
  SerializeFloat(*m, outstr);
}

void tostr(align1_double *m, char *outstr) {
  SerializeDouble(*m, outstr);
}

void tostr(align1_double *m, int numElems, char *outstr) {
  assert(numElems <= 4);
  char s[4][64];
  for(int i = 0; i < numElems; ++i)
    SerializeDouble(m[i], s[i]);
  switch(numElems) {
    case 1: sprintf(outstr, "{%s}", s[0]); break;
    case 2: sprintf(outstr, "{%s,%s}", s[0], s[1]); break;
    case 3:
      sprintf(outstr, "{%s,%s,%s}", s[0], s[1], s[2]);
      break;
    case 4:
      sprintf(outstr, "{%s,%s,%s,%s}", s[0], s[1], s[2], s[3]);
      break;
  }
}

void tostr(align1_float *m, int numElems, char *outstr) {
  assert(numElems <= 8);
  char s[8][64];
  for(int i = 0; i < numElems; ++i)
    SerializeFloat(m[i], s[i]);
  switch(numElems) {
    case 1: sprintf(outstr, "{%s}", s[0]); break;
    case 2: sprintf(outstr, "{%s,%s}", s[0], s[1]); break;
    case 3: sprintf(outstr, "{%s,%s,%s}", s[0], s[1], s[2]); break;
    case 4: sprintf(outstr, "{%s,%s,%s,%s}", s[0], s[1], s[2], s[3]); break;
    case 5:
      sprintf(outstr, "{%s,%s,%s,%s,%s}", s[0], s[1], s[2], s[3], s[4]);
      break;
    case 6:
      sprintf(
        outstr, "{%s,%s,%s,%s,%s,%s}", s[0], s[1], s[2], s[3], s[4], s[5]);
      break;
    case 7:
      sprintf(outstr,
              "{%s,%s,%s,%s,%s,%s,%s}",
              s[0],
              s[1],
              s[2],
              s[3],
              s[4],
              s[5],
              s[6]);
      break;
    case 8:
      sprintf(outstr,
              "{%s,%s,%s,%s,%s,%s,%s,%s}",
              s[0],
              s[1],
              s[2],
              s[3],
              s[4],
              s[5],
              s[6],
              s[7]);
      break;
  }
}

void tostr(align1_int *s, int numElems, char *outstr) {
  assert(numElems <= 8);
  switch(numElems) {
    case 1: sprintf(outstr, "{0x%08X}", s[0]); break;
    case 2: sprintf(outstr, "{0x%08X,0x%08X}", s[0], s[1]); break;
    case 3: sprintf(outstr, "{0x%08X,0x%08X,0x%08X}", s[0], s[1], s[2]); break;
    case 4: sprintf(outstr, "{0x%08X,0x%08X,0x%08X,0x%08X}", s[0], s[1], s[2], s[3]); break;
    case 5:
      sprintf(outstr,
              "{0x%08X,0x%08X,0x%08X,0x%08X,0x%08X}",
              s[0],
              s[1],
              s[2],
              s[3],
              s[4]);
      break;
    case 6:
      sprintf(outstr,
              "{0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X}",
              s[0],
              s[1],
              s[2],
              s[3],
              s[4],
              s[5]);
      break;
    case 7:
      sprintf(outstr,
              "{0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X}",
              s[0],
              s[1],
              s[2],
              s[3],
              s[4],
              s[5],
              s[6]);
      break;
    case 8:
      sprintf(outstr,
              "{0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X}",
              s[0],
              s[1],
              s[2],
              s[3],
              s[4],
              s[5],
              s[6],
              s[7]);
      break;
  }
}

void tostr(align1_int64 *m, int numElems, char *outstr) {
  assert(numElems <= 4);
  switch(numElems) {
    case 1: sprintf(outstr, "{0x%08X%08X}", (int)(*m >> 32), (int)*m); break;
    case 2:
      sprintf(outstr,
              "{0x%08X%08X,0x%08X%08X}",
              (int)(*m >> 32),
              (int)*m,
              (int)(m[1] >> 32),
              (int)m[1]);
      break;
    case 3:
      sprintf(outstr,
              "{0x%08X%08X,0x%08X%08X,0x%08X%08X}",
              (int)(*m >> 32),
              (int)*m,
              (int)(m[1] >> 32),
              (int)m[1],
              (int)(m[2] >> 32),
              (int)m[2]);
      break;
    case 4:
      sprintf(outstr,
              "{0x%08X%08X,0x%08X%08X,0x%08X%08X,0x%08X%08X}",
              (int)(*m >> 32),
              (int)*m,
              (int)(m[1] >> 32),
              (int)m[1],
              (int)(m[2] >> 32),
              (int)m[2],
              (int)(m[3] >> 32),
              (int)m[3]);
      break;
  }
}

// Accessors to the test data in a way that the compiler can't optimize at compile-time.
__attribute__((noinline)) float *get_interesting_floats() {
  return always_true() ? interesting_floats_ : 0;
}

__attribute__((noinline)) uint32_t *get_interesting_ints() {
  return always_true() ? interesting_ints_ : 0;
}

__attribute__((noinline)) double *get_interesting_doubles() {
  return always_true() ? interesting_doubles_ : 0;
}

__m128 ExtractFloatInRandomOrder(float *arr, int i, int n, int prime) {
  return _mm_set_ps(arr[(i*prime)%n], arr[((i+1)*prime)%n], arr[((i+2)*prime)%n], arr[((i+3)*prime)%n]);
}

#ifdef __SSE2__
__m128d ExtractDoubleInRandomOrder(double *arr, int i, int n, int prime) {
  return _mm_set_pd(arr[(i*prime)%n], arr[((i+1)*prime)%n]);
}
#endif

__m128 ExtractIntInRandomOrder(unsigned int *arr, int i, int n, int prime) {
  return _mm_set_ps(*(float*)&arr[(i*prime)%n], *(float*)&arr[((i+1)*prime)%n], *(float*)&arr[((i+2)*prime)%n], *(float*)&arr[((i+3)*prime)%n]);
}

#define E1(arr, i, n) ExtractFloatInRandomOrder(arr, i, n, 1)
#define E2(arr, i, n) ExtractFloatInRandomOrder(arr, i, n, 1787)

#define E1_Double(arr, i, n) ExtractDoubleInRandomOrder(arr, i, n, 1)
#define E2_Double(arr, i, n) ExtractDoubleInRandomOrder(arr, i, n, 1787)

#define E1_Int(arr, i, n) ExtractIntInRandomOrder(arr, i, n, 1)
#define E2_Int(arr, i, n) ExtractIntInRandomOrder(arr, i, n, 1787)

#define M128i_M128i_M128i(func) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingInts / 4; ++j) \
      { \
        __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
        __m128i m2 = (__m128i)E2_Int(interesting_ints, j*4, numInterestingInts); \
        __m128i ret = func(m1, m2); \
        /* a op b */ \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
        /* b op a */ \
        ret = func(m2, m1); \
        tostr(&m1, str); \
        tostr(&m2, str2); \
        tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
    { \
      __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
      Ret_type ret = func(m1, Tint); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s, %d) = %s\n", #func, str, Tint, str2); \
    }

#define Ret_M128d_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
    { \
      __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
      Ret_type ret = func(m1, Tint); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s, %d) = %s\n", #func, str, Tint, str2); \
    }

#define Ret_M128i_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
    { \
      __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
      Ret_type ret = func(m1, Tint); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s, %d) = %s\n", #func, str, Tint, str2); \
    }

#define Ret_M128i_int_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int j = 0; j < numInterestingInts; ++j) \
      for(int k = 0; k < 4; ++k) \
      { \
        __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
        Ret_type ret = func(m1, interesting_ints[j], Tint); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&ret, str2); \
        printf("%s(%s, 0x%08X, %d) = %s\n", #func, str, interesting_ints[j], Tint, str2); \
      }

#define Ret_M128d_M128d_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j < numInterestingDoubles / 2; ++j) \
      { \
        __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
        __m128d m2 = E2_Double(interesting_doubles, j*2, numInterestingDoubles); \
        Ret_type ret = func(m1, m2, Tint); \
        /* a op b */ \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
        /* b op a */ \
        ret = func(m2, m1, Tint); \
        tostr(&m1, str); \
        tostr(&m2, str2); \
        tostr(&ret, str3); \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
      }

#define Ret_M128i_M128i_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingInts / 4; ++j) \
      { \
        __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
        __m128i m2 = (__m128i)E2_Int(interesting_ints, j*4, numInterestingInts); \
        Ret_type ret = func(m1, m2, Tint); \
        /* a op b */ \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
        /* b op a */ \
        ret = func(m2, m1, Tint); \
        tostr(&m1, str); \
        tostr(&m2, str2); \
        tostr(&ret, str3); \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
      }

#define Ret_M128_M128_Tint_body(Ret_type, func, Tint) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingFloats / 4; ++j) \
      { \
        __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
        __m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
        Ret_type ret = func(m1, m2, Tint); \
        /* a op b */ \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
        /* b op a */ \
        ret = func(m2, m1, Tint); \
        tostr(&m1, str); \
        tostr(&m2, str2); \
        tostr(&ret, str3); \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
      }

#define const_int8_unroll(Ret_type, F, func) \
  F(Ret_type, func, 0); \
  F(Ret_type, func, 1); \
  F(Ret_type, func, 2); \
  F(Ret_type, func, 3); \
  F(Ret_type, func, 5); \
  F(Ret_type, func, 7); \
  F(Ret_type, func, 11); \
  F(Ret_type, func, 13); \
  F(Ret_type, func, 15); \
  F(Ret_type, func, 16); \
  F(Ret_type, func, 17); \
  F(Ret_type, func, 23); \
  F(Ret_type, func, 29); \
  F(Ret_type, func, 31); \
  F(Ret_type, func, 37); \
  F(Ret_type, func, 43); \
  F(Ret_type, func, 47); \
  F(Ret_type, func, 59); \
  F(Ret_type, func, 127); \
  F(Ret_type, func, 128); \
  F(Ret_type, func, 191); \
  F(Ret_type, func, 254); \
  F(Ret_type, func, 255);

#define const_int5_full_unroll(Ret_type, F, func) \
  F(Ret_type, func, 0); \
  F(Ret_type, func, 1); \
  F(Ret_type, func, 2); \
  F(Ret_type, func, 3); \
  F(Ret_type, func, 4); \
  F(Ret_type, func, 5); \
  F(Ret_type, func, 6); \
  F(Ret_type, func, 7); \
  F(Ret_type, func, 8); \
  F(Ret_type, func, 9); \
  F(Ret_type, func, 10); \
  F(Ret_type, func, 11); \
  F(Ret_type, func, 12); \
  F(Ret_type, func, 13); \
  F(Ret_type, func, 14); \
  F(Ret_type, func, 15); \
  F(Ret_type, func, 16); \
  F(Ret_type, func, 17); \
  F(Ret_type, func, 18); \
  F(Ret_type, func, 19); \
  F(Ret_type, func, 20); \
  F(Ret_type, func, 21); \
  F(Ret_type, func, 22); \
  F(Ret_type, func, 23); \
  F(Ret_type, func, 24); \
  F(Ret_type, func, 25); \
  F(Ret_type, func, 26); \
  F(Ret_type, func, 27); \
  F(Ret_type, func, 28); \
  F(Ret_type, func, 29); \
  F(Ret_type, func, 30); \
  F(Ret_type, func, 31);

#define Ret_M128_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128_Tint_body, func)
#define Ret_M128d_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128d_Tint_body, func)
#define Ret_M128i_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128i_Tint_body, func)
#define Ret_M128i_int_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128i_int_Tint_body, func)
#define Ret_M128i_M128i_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128i_M128i_Tint_body, func)
#define Ret_M128d_M128d_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128d_M128d_Tint_body, func)
#define Ret_M128d_M128d_Tint_5bits(Ret_type, func) const_int5_full_unroll(Ret_type, Ret_M128d_M128d_Tint_body, func)
#define Ret_M128_M128_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128_M128_Tint_body, func)
#define Ret_M128_M128_Tint_5bits(Ret_type, func) const_int5_full_unroll(Ret_type, Ret_M128_M128_Tint_body, func)

#define Ret_M128d_M128d(Ret_type, func) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j < numInterestingDoubles / 2; ++j) \
      { \
        __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
        __m128d m2 = E2_Double(interesting_doubles, j*2, numInterestingDoubles); \
        Ret_type ret = func(m1, m2); \
        /* a op b */ \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
        /* b op a */ \
        ret = func(m2, m1); \
        tostr(&m1, str); \
        tostr(&m2, str2); \
        tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128d_M128d_M128d(Ret_type, func) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j < numInterestingDoubles / 2; ++j) \
        for(int l = 0; l < numInterestingDoubles / 2; ++l) \
        { \
          __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
          __m128d m2 = E2_Double(interesting_doubles, j*2, numInterestingDoubles); \
          __m128d m3 = E1_Double(interesting_doubles, l*2, numInterestingDoubles); \
          Ret_type ret = func(m1, m2, m3); \
          /* a, b, c */ \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(&m3, str3); \
          char str4[256]; tostr(&ret, str4); \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4); \
          /* b, c, a */ \
          ret = func(m2, m3, m1); \
          tostr(&m1, str); \
          tostr(&m2, str2); \
          tostr(&m3, str3); \
          tostr(&ret, str4); \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4); \
          /* c, a, b */ \
          ret = func(m3, m1, m2); \
          tostr(&m1, str); \
          tostr(&m2, str2); \
          tostr(&m3, str3); \
          tostr(&ret, str4); \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4); \
        }

#define Ret_M128d_M128(Ret_type, func) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j < numInterestingDoubles / 2; ++j) \
      { \
        __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
        __m128 m2 = E2(interesting_floats, i*4+k, numInterestingFloats); \
        Ret_type ret = func(m1, m2); \
        /* a op b */ \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128d_int(Ret_type, func) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j < numInterestingInts; ++j) \
      { \
        __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
        int m2 = interesting_ints[j]; \
        Ret_type ret = func(m1, m2); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128d_int64(Ret_type, func) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j < numInterestingInts; ++j) \
        for(int l = 0; l < numInterestingInts; ++l) \
        { \
          __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
          int64_t m2 = (int64_t)(((uint64_t)interesting_ints[j]) << 32 | (uint64_t)interesting_ints[l]); \
          Ret_type ret = func(m1, m2); \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(&ret, str3); \
          printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
        }

#define Ret_M128d(Ret_type, func) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
    { \
      __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
      Ret_type ret = func(m1); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

#define Ret_DoublePtr(Ret_type, func, numElemsAccessed, inc) \
  for(int i = 0; i+numElemsAccessed <= numInterestingDoubles; i += inc) \
  { \
    double *ptr = interesting_doubles + i; \
    Ret_type ret = func(ptr); \
    char str[256]; tostr(ptr, numElemsAccessed, str); \
    char str2[256]; tostr(&ret, str2); \
    printf("%s(%s) = %s\n", #func, str, str2); \
  }

#define Ret_DoublePtr_M128i(Ret_type, func, numElemsAccessed, inc) \
  for(int i = 0; i+numElemsAccessed <= numInterestingDoubles; i += inc) \
    for(int j = 0; j < numInterestingInts / 4; ++j) \
    { \
      double *ptr = interesting_doubles + i; \
      __m128i m1 = (__m128i)E2_Int(interesting_ints, j*4, numInterestingInts); \
      Ret_type ret = func(ptr, m1); \
      char str[256]; tostr(ptr, numElemsAccessed, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

float tempOutFloatStore[32];
float *getTempOutFloatStore(int alignmentBytes) {
  memset(tempOutFloatStore, 0, sizeof(tempOutFloatStore));
  uintptr_t addr = (uintptr_t)tempOutFloatStore;
  addr = (addr + alignmentBytes - 1) & ~(alignmentBytes-1);
  return (float*)addr;
}

int *getTempOutIntStore(int alignmentBytes) { return (int*)getTempOutFloatStore(alignmentBytes); }
double *getTempOutDoubleStore(int alignmentBytes) { return (double*)getTempOutFloatStore(alignmentBytes); }

#define void_OutFloatPtr_M128(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
      for(int k = 0; k < 4; ++k) \
      { \
        uintptr_t base = (uintptr_t)getTempOutFloatStore(16); \
        __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
        align1_float *out = (align1_float*)(base + offset); \
        func((Ptr_type)out, m1); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(out, numBytesWritten/sizeof(float), str2); \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
      }

#define void_OutFloatPtr_M128i_M128(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int j = 0; j < numInterestingInts / 4; ++j) \
      for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
        for(int k = 0; k < 4; ++k) \
        { \
          uintptr_t base = (uintptr_t)getTempOutFloatStore(16); \
          __m128i m1 = (__m128i)E1_Int(interesting_ints, j*4, numInterestingInts); \
          __m128 m2 = E1(interesting_floats, i*4+k, numInterestingFloats); \
          align1_float *out = (align1_float*)(base + offset); \
          func((Ptr_type)out, m1, m2); \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(out, numBytesWritten/sizeof(float), str3); \
          printf("%s(p:align=%d, %s, %s) = %s\n", #func, offset, str, str2, str3); \
        }

#define void_OutDoublePtr_M128d(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
      for(int k = 0; k < 2; ++k) \
      { \
        uintptr_t base = (uintptr_t)getTempOutDoubleStore(16); \
        __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
        align1_double *out = (align1_double*)(base + offset); \
        func((Ptr_type)out, m1); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(out, numBytesWritten/sizeof(double), str2); \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
      }

#define void_OutDoublePtr_M128i_M128d(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int j = 0; j < numInterestingInts / 4; ++j) \
      for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
        for(int k = 0; k < 2; ++k) \
        { \
          uintptr_t base = (uintptr_t)getTempOutDoubleStore(16); \
          __m128i m1 = (__m128i)E1_Int(interesting_ints, j*4, numInterestingInts); \
          __m128d m2 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
          align1_double *out = (align1_double*)(base + offset); \
          func((Ptr_type)out, m1, m2); \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(out, numBytesWritten/sizeof(double), str3); \
          printf("%s(p:align=%d, %s, %s) = %s\n", #func, offset, str, str2, str3); \
        }

#define void_OutIntPtr_M128i(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
      for(int k = 0; k < 4; ++k) \
      { \
        uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
        __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
        align1_int *out = (align1_int*)(base + offset); \
        func((Ptr_type)out, m1); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(out, (numBytesWritten+sizeof(int)-1)/sizeof(int), str2); \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
      }

#define void_OutIntPtr_int(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingInts; ++i) \
    for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
      for(int k = 0; k < 4; ++k) \
      { \
        uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
        int m1 = interesting_ints[i]; \
        align1_int *out = (align1_int*)(base + offset); \
        func((Ptr_type)out, m1); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(out, numBytesWritten/sizeof(int), str2); \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
      }

#define void_OutIntPtr_int64(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingInts; ++i) \
    for(int j = 0; j < numInterestingInts; ++j) \
      for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
      { \
        uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
        int64_t m1 = (int64_t)(((uint64_t)interesting_ints[i]) << 32 | (uint64_t)interesting_ints[j]); \
        align1_int64 *out = (align1_int64*)(base + offset); \
        func((Ptr_type)out, m1); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(out, numBytesWritten/sizeof(int64_t), str2); \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
      }

#define void_M128i_M128i_OutIntPtr(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int j = 0; j < numInterestingInts / 4; ++j) \
      for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
        for(int k = 0; k < 4; ++k) \
        { \
          uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
          __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
          __m128i m2 = (__m128i)E2_Int(interesting_ints, j*4, numInterestingInts); \
          align1_int *out = (int*)(base + offset); \
          func(m1, m2, (Ptr_type)out); \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(out, numBytesWritten/sizeof(int), str3); \
          printf("%s(%s, %s, p:align=%d) = %s\n", #func, str, str2, offset, str3); \
        }

#define Ret_M128(Ret_type, func) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
    { \
      __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
      Ret_type ret = func(m1); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

#define Ret_M128approx(Ret_type, func) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
    { \
      __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
      Ret_type ret = func(m1); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr_approx(&ret, str2, true/*approximate*/); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

#define Ret_FloatPtr(Ret_type, func, numElemsAccessed, inc) \
  for(int i = 0; i+numElemsAccessed <= numInterestingFloats; i += inc) \
  { \
    float *ptr = interesting_floats + i; \
    Ret_type ret = func(ptr); \
    char str[256]; tostr(ptr, numElemsAccessed, str); \
    char str2[256]; tostr(&ret, str2); \
    printf("%s(%s) = %s\n", #func, str, str2); \
  }

#define Ret_FloatPtr_M128i(Ret_type, func, numElemsAccessed, inc) \
  for(int i = 0; i+numElemsAccessed <= numInterestingFloats; i += inc) \
    for(int j = 0; j < numInterestingInts / 4; ++j) \
    { \
      float *ptr = interesting_floats + i; \
      __m128i m1 = (__m128i)E1_Int(interesting_ints, j*4, numInterestingInts); \
      Ret_type ret = func(ptr, m1); \
      char str[256]; tostr(ptr, numElemsAccessed, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

#define Ret_Float4(Ret_type, func, inc) \
  for(int i = 0; i+4 <= numInterestingFloats; i += inc) \
  { \
    float *ptr = interesting_floats + i; \
    Ret_type ret = func(ptr[0], ptr[1], ptr[2], ptr[3]); \
    char str[256]; tostr(ptr, 4, str); \
    char str2[256]; tostr(&ret, str2); \
    printf("%s(%s) = %s\n", #func, str, str2); \
  }

#define Ret_Float(Ret_type, func, inc) \
  for(int i = 0; i+1 <= numInterestingFloats; i += inc) \
  { \
    float *ptr = interesting_floats + i; \
    Ret_type ret = func(*ptr); \
    char str[256]; tostr(ptr, 1, str); \
    char str2[256]; tostr(&ret, str2); \
    printf("%s(%s) = %s\n", #func, str, str2); \
  }

#define Ret_IntPtr(Ret_type, func, Ptr_type, numElemsAccessed, inc) \
  for(int i = 0; i+numElemsAccessed <= numInterestingInts; i += inc) \
  { \
    uint32_t *ptr = interesting_ints + i; \
    Ret_type ret = func((Ptr_type)ptr); \
    char str[256]; tostr((int*)ptr, numElemsAccessed, str); \
    char str2[256]; tostr(&ret, str2); \
    printf("%s(%s) = %s\n", #func, str, str2); \
  }

#define Ret_M128_FloatPtr(Ret_type, func, Ptr_type, numElemsAccessed, inc) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j+numElemsAccessed <= numInterestingFloats; j += inc) \
      { \
        __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
        float *ptr = interesting_floats + j; \
        Ret_type ret = func(m1, (Ptr_type)ptr); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(ptr, numElemsAccessed, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128d_DoublePtr(Ret_type, func, Ptr_type, numElemsAccessed, inc) \
  for(int i = 0; i < numInterestingDoubles / 2; ++i) \
    for(int k = 0; k < 2; ++k) \
      for(int j = 0; j+numElemsAccessed <= numInterestingDoubles; j += inc) \
      { \
        __m128d m1 = E1_Double(interesting_doubles, i*2+k, numInterestingDoubles); \
        double *ptr = interesting_doubles + j; \
        Ret_type ret = func(m1, (Ptr_type)ptr); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(ptr, numElemsAccessed, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128i(Ret_type, func) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
    { \
      __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
      Ret_type ret = func(m1); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

#define Ret_M128i_M128i(Ret_type, func) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingInts / 4; ++j) \
      { \
        __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
        __m128i m2 = (__m128i)E2_Int(interesting_ints, j*4, numInterestingInts); \
        Ret_type ret = func(m1, m2); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128i_M128i_M128i(Ret_type, func) \
  for(int i = 0; i < numInterestingInts / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingInts / 4; ++j) \
        for(int l = 0; l < numInterestingInts / 4; ++l) \
        { \
          __m128i m1 = (__m128i)E1_Int(interesting_ints, i*4+k, numInterestingInts); \
          __m128i m2 = (__m128i)E2_Int(interesting_ints, j*4, numInterestingInts); \
          __m128i m3 = (__m128i)E1_Int(interesting_ints, l*4, numInterestingInts); \
          Ret_type ret = func(m1, m2, m3); \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(&m3, str3); \
          char str4[256]; tostr(&ret, str4); \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4); \
        }

#define Ret_int(Ret_type, func) \
  for(int i = 0; i < numInterestingInts; ++i) \
  { \
    Ret_type ret = func(interesting_ints[i]); \
    char str[256]; tostr((int*)&interesting_ints[i], str); \
    char str2[256]; tostr(&ret, str2); \
    printf("%s(%s) = %s\n", #func, str, str2); \
  }

#define Ret_int64(Ret_type, func) \
  for(int i = 0; i < numInterestingInts; ++i) \
    for(int j = 0; j < numInterestingInts; ++j) \
    { \
      int64_t m1 = (int64_t)(((uint64_t)interesting_ints[i]) << 32 | (uint64_t)interesting_ints[j]); \
      Ret_type ret = func(m1); \
      char str[256]; tostr(&m1, str); \
      char str2[256]; tostr(&ret, str2); \
      printf("%s(%s) = %s\n", #func, str, str2); \
    }

#define Ret_M128_M128(Ret_type, func) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingFloats / 4; ++j) \
      { \
        __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
        __m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
        Ret_type ret = func(m1, m2); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#define Ret_M128_M128_M128(Ret_type, func) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingFloats / 4; ++j) \
        for(int l = 0; l < numInterestingFloats / 4; ++l) \
        { \
          __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
          __m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
          __m128 m3 = E1(interesting_floats, l*4, numInterestingFloats); \
          Ret_type ret = func(m1, m2, m3); \
          char str[256]; tostr(&m1, str); \
          char str2[256]; tostr(&m2, str2); \
          char str3[256]; tostr(&m3, str3); \
          char str4[256]; tostr(&ret, str4); \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4); \
        }

#define Ret_M128_int(Ret_type, func) \
  for(int i = 0; i < numInterestingFloats / 4; ++i) \
    for(int k = 0; k < 4; ++k) \
      for(int j = 0; j < numInterestingInts; ++j) \
      { \
        __m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
        int m2 = interesting_ints[j]; \
        Ret_type ret = func(m1, m2); \
        char str[256]; tostr(&m1, str); \
        char str2[256]; tostr(&m2, str2); \
        char str3[256]; tostr(&ret, str3); \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
      }

#ifdef __AVX__

void tostr(__m256* m, char* outstr) {
  union {
    __m256 m;
    float val[8];
  } u;
  u.m = *m;
  char s[8][32];
  for (int i = 0; i < 8; i++) {
    SerializeFloat(u.val[i], s[i]);
  }
  sprintf(outstr,
          "[%s,%s,%s,%s,%s,%s,%s,%s]",
          s[7],
          s[6],
          s[5],
          s[4],
          s[3],
          s[2],
          s[1],
          s[0]);
}

void tostr(__m256i* m, char* outstr) {
  union {
    __m256i m;
    uint32_t val[8];
  } u;
  u.m = *m;
  sprintf(outstr,
          "[0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X,0x%08X]",
          u.val[7],
          u.val[6],
          u.val[5],
          u.val[4],
          u.val[3],
          u.val[2],
          u.val[1],
          u.val[0]);
}

void tostr(__m256d* m, char* outstr) {
  union {
    __m256d m;
    double val[4];
  } u;
  u.m = *m;
  char s[4][64];
  SerializeDouble(u.val[0], s[0]);
  SerializeDouble(u.val[1], s[1]);
  SerializeDouble(u.val[2], s[2]);
  SerializeDouble(u.val[3], s[3]);
  sprintf(outstr, "[%s,%s,%s,%s]", s[3], s[2], s[1], s[0]);
}

void tostr_approx(__m256* m, char* outstr, bool approximate) {
  union {
    __m256 m;
    float val[8];
  } u;
  u.m = *m;
  char s[8][32];

  for (int i = 0; i < 8; i++) {
    SerializeFloat(u.val[i], s[i], approximate);
  }
  sprintf(outstr,
          "[%s,%s,%s,%s,%s,%s,%s,%s]",
          s[7],
          s[6],
          s[5],
          s[4],
          s[3],
          s[2],
          s[1],
          s[0]);
}

#define Ret_M128_M128i(Ret_type, func)                                         \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128 m1 = E1(interesting_floats, i * 4 + k, numInterestingFloats);   \
        __m128i m2 =                                                           \
          (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);        \
        Ret_type ret = func(m1, m2);                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M128d_M128i(Ret_type, func)                                        \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128d m1 =                                                           \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m128i m2 =                                                           \
          (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);        \
        Ret_type ret = func(m1, m2);                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M256d(Ret_type, func)                                              \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k) {                                              \
      __m128d tmp =                                                            \
        E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);      \
      __m256d m1 = _mm256_set_m128d(tmp, tmp);                                 \
      Ret_type ret = func(m1);                                                 \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s) = %s\n", #func, str, str2);                               \
    }

#define Ret_M256(Ret_type, func)                                               \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k) {                                              \
      __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);    \
      __m256 m1 = _mm256_set_m128(tmp, tmp);                                   \
      Ret_type ret = func(m1);                                                 \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s) = %s\n", #func, str, str2);                               \
    }

#define Ret_M256approx(Ret_type, func)                                         \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k) {                                              \
      __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);    \
      __m256 m1 = _mm256_set_m128(tmp, tmp);                                   \
      Ret_type ret = func(m1);                                                 \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr_approx(&ret, str2, true /*approximate*/);                          \
      printf("%s(%s) = %s\n", #func, str, str2);                               \
    }

#define Ret_M256d_M256d(Ret_type, func)                                        \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k)                                                \
      for (int j = 0; j < numInterestingDoubles / 2; ++j) {                    \
        __m128d tmp =                                                          \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m256d m1 = _mm256_set_m128d(tmp, tmp);                               \
        tmp = E2_Double(interesting_doubles, j * 2, numInterestingDoubles);    \
        __m256d m2 = _mm256_set_m128d(tmp, tmp);                               \
        Ret_type ret = func(m1, m2);                                           \
        /* a op b */                                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
        /* b op a */                                                           \
        ret = func(m2, m1);                                                    \
        tostr(&m1, str);                                                       \
        tostr(&m2, str2);                                                      \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M256_M256(Ret_type, func)                                          \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingFloats / 4; ++j) {                     \
        __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);  \
        __m256 m1 = _mm256_set_m128(tmp, tmp);                                 \
        tmp = E2(interesting_floats, j * 4, numInterestingFloats);             \
        __m256 m2 = _mm256_set_m128(tmp, tmp);                                 \
        Ret_type ret = func(m1, m2);                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M256i_M256i(Ret_type, func)                                        \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128i tmp =                                                          \
          (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);    \
        __m256i m1 = _mm256_set_m128i(tmp, tmp);                               \
        tmp = (__m128i)E2_Int(interesting_ints, j * 4, numInterestingInts);    \
        __m256i m2 = _mm256_set_m128i(tmp, tmp);                               \
        Ret_type ret = func(m1, m2);                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M256d_M256i(Ret_type, func)                                        \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128d tmp1 =                                                         \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m256d m1 = _mm256_set_m128d(tmp1, tmp1);                             \
        __m128i tmp2 =                                                         \
          (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);        \
        __m256i m2 = _mm256_set_m128i(tmp2, tmp2);                             \
        Ret_type ret = func(m1, m2);                                           \
        /* a op b */                                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M256_M256i(Ret_type, func)                                         \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128 tmp1 = E1(interesting_floats, i * 4 + k, numInterestingFloats); \
        __m256 m1 = _mm256_set_m128(tmp1, tmp1);                               \
        __m128i tmp2 =                                                         \
          (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);        \
        __m256i m2 = _mm256_set_m128i(tmp2, tmp2);                             \
        Ret_type ret = func(m1, m2);                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s) = %s\n", #func, str, str2, str3);                   \
      }

#define Ret_M256_M256_M256(Ret_type, func)                                     \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingFloats / 4; ++j)                       \
        for (int l = 0; l < numInterestingFloats / 4; ++l) {                   \
          __m128 tmp =                                                         \
            E1(interesting_floats, i * 4 + k, numInterestingFloats);           \
          __m256 m1 = _mm256_set_m128(tmp, tmp);                               \
          tmp = E2(interesting_floats, j * 4, numInterestingFloats);           \
          __m256 m2 = _mm256_set_m128(tmp, tmp);                               \
          tmp = E1(interesting_floats, l * 4, numInterestingFloats);           \
          __m256 m3 = _mm256_set_m128(tmp, tmp);                               \
          Ret_type ret = func(m1, m2, m3);                                     \
          char str[256];                                                       \
          tostr(&m1, str);                                                     \
          char str2[256];                                                      \
          tostr(&m2, str2);                                                    \
          char str3[256];                                                      \
          tostr(&m3, str3);                                                    \
          char str4[256];                                                      \
          tostr(&ret, str4);                                                   \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4);       \
        }

#define Ret_M256d_M256d_M256d(Ret_type, func)                                  \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k)                                                \
      for (int j = 0; j < numInterestingDoubles / 2; ++j)                      \
        for (int l = 0; l < numInterestingDoubles / 2; ++l) {                  \
          __m128d tmp =                                                        \
            E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);  \
          __m256d m1 = _mm256_set_m128d(tmp, tmp);                             \
          tmp = E2_Double(interesting_doubles, j * 2, numInterestingDoubles);  \
          __m256d m2 = _mm256_set_m128d(tmp, tmp);                             \
          tmp = E1_Double(interesting_doubles, l * 2, numInterestingDoubles);  \
          __m256d m3 = _mm256_set_m128d(tmp, tmp);                             \
          Ret_type ret = func(m1, m2, m3);                                     \
          /* a, b, c */                                                        \
          char str[256];                                                       \
          tostr(&m1, str);                                                     \
          char str2[256];                                                      \
          tostr(&m2, str2);                                                    \
          char str3[256];                                                      \
          tostr(&m3, str3);                                                    \
          char str4[256];                                                      \
          tostr(&ret, str4);                                                   \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4);       \
          /* b, c, a */                                                        \
          ret = func(m2, m3, m1);                                              \
          tostr(&m1, str);                                                     \
          tostr(&m2, str2);                                                    \
          tostr(&m3, str3);                                                    \
          tostr(&ret, str4);                                                   \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4);       \
          /* c, a, b */                                                        \
          ret = func(m3, m1, m2);                                              \
          tostr(&m1, str);                                                     \
          tostr(&m2, str2);                                                    \
          tostr(&m3, str3);                                                    \
          tostr(&ret, str4);                                                   \
          printf("%s(%s, %s, %s) = %s\n", #func, str, str2, str3, str4);       \
        }

#define Ret_M256i(Ret_type, func)                                              \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int k = 0; k < 4; ++k) {                                              \
      __m128i tmp =                                                            \
        (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);      \
      __m256i m1 = _mm256_set_m128i(tmp, tmp);                                 \
      Ret_type ret = func(m1);                                                 \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s) = %s\n", #func, str, str2);                               \
    }

#define Ret_M128dPtr(Ret_type, func)                                           \
  for (int i = 0; i + 2 <= numInterestingDoubles; i += 2) {                    \
    double* ptr = interesting_doubles + i;                                     \
    Ret_type ret = func((__m128d*)ptr);                                        \
    char str[256];                                                             \
    tostr(ptr, 2, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_M128Ptr(Ret_type, func)                                            \
  for (int i = 0; i + 4 <= numInterestingFloats; i += 4) {                     \
    float* ptr = interesting_floats + i;                                       \
    Ret_type ret = func((__m128*)ptr);                                         \
    char str[256];                                                             \
    tostr(ptr, 4, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_DoublePtr_DoublePtr(Ret_type, func, numElemsAccessed, inc)         \
  for (int i = 0; i + numElemsAccessed <= numInterestingDoubles; i += inc) {   \
    double* ptr1 = interesting_doubles + i;                                    \
    for (int j = 0; j + numElemsAccessed <= numInterestingDoubles; j += inc) { \
      double* ptr2 = interesting_doubles + j;                                  \
      Ret_type ret = func(ptr1, ptr2);                                         \
      char str1[256];                                                          \
      tostr(ptr1, numElemsAccessed, str1);                                     \
      char str2[256];                                                          \
      tostr(ptr2, numElemsAccessed, str2);                                     \
      char str3[256];                                                          \
      tostr(&ret, str3);                                                       \
      printf("%s(%s, %s) = %s\n", #func, str1, str2, str3);                    \
    }                                                                          \
  }

#define Ret_FloatPtr_FloatPtr(Ret_type, func, numElemsAccessed, inc)           \
  for (int i = 0; i + numElemsAccessed <= numInterestingFloats; i += inc) {    \
    float* ptr1 = interesting_floats + i;                                      \
    for (int j = 0; j + numElemsAccessed <= numInterestingFloats; j += inc) {  \
      float* ptr2 = interesting_floats + j;                                    \
      Ret_type ret = func(ptr1, ptr2);                                         \
      char str1[256];                                                          \
      tostr(ptr1, numElemsAccessed, str1);                                     \
      char str2[256];                                                          \
      tostr(ptr2, numElemsAccessed, str2);                                     \
      char str3[256];                                                          \
      tostr(&ret, str3);                                                       \
      printf("%s(%s,%s) = %s\n", #func, str1, str2, str3);                     \
    }                                                                          \
  }

#define Ret_IntPtr_IntPtr(Ret_type, func, Ptr_type, numElemsAccessed, inc)     \
  for (int i = 0; i + numElemsAccessed <= numInterestingInts; i += inc) {      \
    uint32_t* ptr1 = interesting_ints + i;                                     \
    for (int j = 0; j + numElemsAccessed <= numInterestingInts; j += inc) {    \
      uint32_t* ptr2 = interesting_ints + j;                                   \
      Ret_type ret = func((Ptr_type)ptr1, (Ptr_type)ptr2);                     \
      char str1[256];                                                          \
      tostr((int*)ptr1, numElemsAccessed, str1);                               \
      char str2[256];                                                          \
      tostr((int*)ptr2, numElemsAccessed, str2);                               \
      char str3[256];                                                          \
      tostr(&ret, str3);                                                       \
      printf("%s(%s, %s) = %s\n", #func, str1, str2, str3);                    \
    }                                                                          \
  }

#define Ret_DoublePtr_M256i(Ret_type, func, numElemsAccessed, inc)             \
  for (int i = 0; i + numElemsAccessed <= numInterestingDoubles; i += inc)     \
    for (int j = 0; j < numInterestingInts / 4; ++j) {                         \
      double* ptr = interesting_doubles + i;                                   \
      __m128i tmp =                                                            \
        (__m128i)E2_Int(interesting_ints, j * 4, numInterestingInts);          \
      __m256i m1 = _mm256_set_m128i(tmp, tmp);                                 \
      Ret_type ret = func(ptr, m1);                                            \
      char str[256];                                                           \
      tostr(ptr, numElemsAccessed, str);                                       \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s) = %s\n", #func, str, str2);                               \
    }

#define Ret_FloatPtr_M256i(Ret_type, func, numElemsAccessed, inc)              \
  for (int i = 0; i + numElemsAccessed <= numInterestingFloats; i += inc)      \
    for (int j = 0; j < numInterestingInts / 4; ++j) {                         \
      float* ptr = interesting_floats + i;                                     \
      __m128i tmp =                                                            \
        (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);          \
      __m256i m1 = _mm256_set_m128i(tmp, tmp);                                 \
      Ret_type ret = func(ptr, m1);                                            \
      char str[256];                                                           \
      tostr(ptr, numElemsAccessed, str);                                       \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s) = %s\n", #func, str, str2);                               \
    }

#define Ret_M256d_M256d_Tint_body(Ret_type, func, Tint)                        \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k)                                                \
      for (int j = 0; j < numInterestingDoubles / 2; ++j) {                    \
        __m128d tmp =                                                          \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m256d m1 = _mm256_set_m128d(tmp, tmp);                               \
        tmp = E2_Double(interesting_doubles, j * 2, numInterestingDoubles);    \
        __m256d m2 = _mm256_set_m128d(tmp, tmp);                               \
        Ret_type ret = func(m1, m2, Tint);                                     \
        /* a op b */                                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
        /* b op a */                                                           \
        ret = func(m2, m1, Tint);                                              \
        tostr(&m1, str);                                                       \
        tostr(&m2, str2);                                                      \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
      }

#define Ret_M256_M256_Tint_body(Ret_type, func, Tint)                          \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingFloats / 4; ++j) {                     \
        __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);  \
        __m256 m1 = _mm256_set_m128(tmp, tmp);                                 \
        tmp = E2(interesting_floats, j * 4, numInterestingFloats);             \
        __m256 m2 = _mm256_set_m128(tmp, tmp);                                 \
        Ret_type ret = func(m1, m2, Tint);                                     \
        /* a op b */                                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
        /* b op a */                                                           \
        ret = func(m2, m1, Tint);                                              \
        tostr(&m1, str);                                                       \
        tostr(&m2, str2);                                                      \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
      }

#define Ret_M256i_M256i_Tint_body(Ret_type, func, Tint)                        \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128i tmp =                                                          \
          (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);    \
        __m256i m1 = _mm256_set_m128i(tmp, tmp);                               \
        tmp = (__m128i)E2_Int(interesting_ints, j * 4, numInterestingInts);    \
        __m256i m2 = _mm256_set_m128i(tmp, tmp);                               \
        Ret_type ret = func(m1, m2, Tint);                                     \
        /* a op b */                                                           \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
        /* b op a */                                                           \
        ret = func(m2, m1, Tint);                                              \
        tostr(&m1, str);                                                       \
        tostr(&m2, str2);                                                      \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
      }

#define Ret_M256_Tint_body(Ret_type, func, Tint)                               \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k) {                                              \
      __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);    \
      __m256 m1 = _mm256_set_m128(tmp, tmp);                                   \
      Ret_type ret = func(m1, Tint);                                           \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s, %d) = %s\n", #func, str, Tint, str2);                     \
    }

#define Ret_M256d_Tint_body(Ret_type, func, Tint)                              \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k) {                                              \
      __m128d tmp =                                                            \
        E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);      \
      __m256d m1 = _mm256_set_m128d(tmp, tmp);                                 \
      Ret_type ret = func(m1, Tint);                                           \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s, %d) = %s\n", #func, str, Tint, str2);                     \
    }

#define Ret_M256i_Tint_body(Ret_type, func, Tint)                              \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int k = 0; k < 4; ++k) {                                              \
      __m128i tmp =                                                            \
        (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);      \
      __m256i m1 = _mm256_set_m128i(tmp, tmp);                                 \
      Ret_type ret = func(m1, Tint);                                           \
      char str[256];                                                           \
      tostr(&m1, str);                                                         \
      char str2[256];                                                          \
      tostr(&ret, str2);                                                       \
      printf("%s(%s, %d) = %s\n", #func, str, Tint, str2);                     \
    }

#define Ret_M256i_int_Tint_body(Ret_type, func, Tint)                          \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int j = 0; j < numInterestingInts; ++j)                               \
      for (int k = 0; k < 4; ++k) {                                            \
        __m128i tmp =                                                          \
          (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);    \
        __m256i m1 = _mm256_set_m128i(tmp, tmp);                               \
        Ret_type ret = func(m1, interesting_ints[j], Tint);                    \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&ret, str2);                                                     \
        printf("%s(%s, 0x%08X, %d) = %s\n",                                    \
               #func,                                                          \
               str,                                                            \
               interesting_ints[j],                                            \
               Tint,                                                           \
               str2);                                                          \
      }

#define Ret_M256i_M128i_Tint_body(Ret_type, func, Tint)                        \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingInts / 4; ++j) {                       \
        __m128i tmp =                                                          \
          (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);    \
        __m256i m1 = _mm256_set_m128i(tmp, tmp);                               \
        __m128i m2 =                                                           \
          (__m128i)E2_Int(interesting_ints, j * 4, numInterestingInts);        \
        Ret_type ret = func(m1, m2, Tint);                                     \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
      }

#define Ret_M256d_M128d_Tint_body(Ret_type, func, Tint)                        \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int k = 0; k < 2; ++k)                                                \
      for (int j = 0; j < numInterestingDoubles / 2; ++j) {                    \
        __m128d tmp =                                                          \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m256d m1 = _mm256_set_m128d(tmp, tmp);                               \
        __m128d m2 =                                                           \
          E2_Double(interesting_doubles, j * 2, numInterestingDoubles);        \
        Ret_type ret = func(m1, m2, Tint);                                     \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
      }

#define Ret_M256_M128_Tint_body(Ret_type, func, Tint)                          \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int k = 0; k < 4; ++k)                                                \
      for (int j = 0; j < numInterestingFloats / 4; ++j) {                     \
        __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);  \
        __m256 m1 = _mm256_set_m128(tmp, tmp);                                 \
        __m128 m2 = E2(interesting_floats, j * 4, numInterestingFloats);       \
        Ret_type ret = func(m1, m2, Tint);                                     \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(&m2, str2);                                                      \
        char str3[256];                                                        \
        tostr(&ret, str3);                                                     \
        printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3);         \
      }

#define Ret_M256_Tint(Ret_type, func)                                          \
  const_int8_unroll(Ret_type, Ret_M256_Tint_body, func)
#define Ret_M256d_Tint(Ret_type, func)                                         \
  const_int8_unroll(Ret_type, Ret_M256d_Tint_body, func)

#define Ret_M256i_M256i_Tint(Ret_type, func)                                   \
  const_int8_unroll(Ret_type, Ret_M256i_M256i_Tint_body, func)
#define Ret_M256d_M256d_Tint(Ret_type, func)                                   \
  const_int8_unroll(Ret_type, Ret_M256d_M256d_Tint_body, func)
#define Ret_M256_M256_Tint(Ret_type, func)                                     \
  const_int8_unroll(Ret_type, Ret_M256_M256_Tint_body, func)
#define Ret_M256i_Tint(Ret_type, func)                                         \
  const_int8_unroll(Ret_type, Ret_M256i_Tint_body, func)

#define Ret_M256i_int_Tint(Ret_type, func)                                     \
  const_int8_unroll(Ret_type, Ret_M256i_int_Tint_body, func)

#define Ret_M256i_M128i_Tint(Ret_type, func)                                   \
  const_int8_unroll(Ret_type, Ret_M256i_M128i_Tint_body, func)
#define Ret_M256d_M128d_Tint(Ret_type, func)                                   \
  const_int8_unroll(Ret_type, Ret_M256d_M128d_Tint_body, func)
#define Ret_M256_M128_Tint(Ret_type, func)                                     \
  const_int8_unroll(Ret_type, Ret_M256_M128_Tint_body, func)

#define const_int5_unroll_0_to_15(Ret_type, F, func)                           \
  F(Ret_type, func, 0);                                                        \
  F(Ret_type, func, 1);                                                        \
  F(Ret_type, func, 2);                                                        \
  F(Ret_type, func, 3);                                                        \
  F(Ret_type, func, 4);                                                        \
  F(Ret_type, func, 5);                                                        \
  F(Ret_type, func, 6);                                                        \
  F(Ret_type, func, 7);                                                        \
  F(Ret_type, func, 8);                                                        \
  F(Ret_type, func, 9);                                                        \
  F(Ret_type, func, 10);                                                       \
  F(Ret_type, func, 11);                                                       \
  F(Ret_type, func, 12);                                                       \
  F(Ret_type, func, 13);                                                       \
  F(Ret_type, func, 14);                                                       \
  F(Ret_type, func, 15);

#define const_int5_unroll_16_to_31(Ret_type, F, func)                          \
  F(Ret_type, func, 16);                                                       \
  F(Ret_type, func, 17);                                                       \
  F(Ret_type, func, 18);                                                       \
  F(Ret_type, func, 19);                                                       \
  F(Ret_type, func, 20);                                                       \
  F(Ret_type, func, 21);                                                       \
  F(Ret_type, func, 22);                                                       \
  F(Ret_type, func, 23);                                                       \
  F(Ret_type, func, 24);                                                       \
  F(Ret_type, func, 25);                                                       \
  F(Ret_type, func, 26);                                                       \
  F(Ret_type, func, 27);                                                       \
  F(Ret_type, func, 28);                                                       \
  F(Ret_type, func, 29);                                                       \
  F(Ret_type, func, 30);                                                       \
  F(Ret_type, func, 31);

#define Ret_M256d_M256d_Tint_5bits_0_to_15(Ret_type, func)                     \
  const_int5_unroll_0_to_15(Ret_type, Ret_M256d_M256d_Tint_body, func)
#define Ret_M256d_M256d_Tint_5bits_16_to_31(Ret_type, func)                    \
  const_int5_unroll_16_to_31(Ret_type, Ret_M256d_M256d_Tint_body, func)

#define Ret_M256_M256_Tint_5bits_0_to_15(Ret_type, func)                       \
  const_int5_unroll_0_to_15(Ret_type, Ret_M256_M256_Tint_body, func)
#define Ret_M256_M256_Tint_5bits_16_to_31(Ret_type, func)                      \
  const_int5_unroll_16_to_31(Ret_type, Ret_M256_M256_Tint_body, func)

#define void_OutDoublePtr_M256d(                                               \
  func, Ptr_type, numBytesWritten, alignmentBytes)                             \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes)   \
      for (int k = 0; k < 2; ++k) {                                            \
        uintptr_t base = (uintptr_t)getTempOutDoubleStore(32);                 \
        __m128d tmp =                                                          \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m256d m1 = _mm256_set_m128d(tmp, tmp);                               \
        align1_double* out = (align1_double*)(base + offset);                  \
        func((Ptr_type)out, m1);                                               \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(out, numBytesWritten / sizeof(double), str2);                    \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2);         \
      }

#define void_OutFloatPtr_M256(func, Ptr_type, numBytesWritten, alignmentBytes) \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes)   \
      for (int k = 0; k < 4; ++k) {                                            \
        uintptr_t base = (uintptr_t)getTempOutFloatStore(32);                  \
        __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);  \
        __m256 m1 = _mm256_set_m128(tmp, tmp);                                 \
        align1_float* out = (align1_float*)(base + offset);                    \
        func((Ptr_type)out, m1);                                               \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(out, numBytesWritten / sizeof(float), str2);                     \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2);         \
      }

#define void_OutIntPtr_M256i(func, Ptr_type, numBytesWritten, alignmentBytes)  \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes)   \
      for (int k = 0; k < 4; ++k) {                                            \
        uintptr_t base = (uintptr_t)getTempOutIntStore(32);                    \
        __m128i tmp =                                                          \
          (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);    \
        __m256i m1 = _mm256_set_m128i(tmp, tmp);                               \
        align1_int* out = (align1_int*)(base + offset);                        \
        func((Ptr_type)out, m1);                                               \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(out, (numBytesWritten + sizeof(int) - 1) / sizeof(int), str2);   \
        printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2);         \
      }

#define void_OutDoublePtr_M256i_M256d(                                         \
  func, Ptr_type, numBytesWritten, alignmentBytes)                             \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int j = 0; j < numInterestingInts / 4; ++j)                           \
      for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
        for (int k = 0; k < 2; ++k) {                                          \
          uintptr_t base = (uintptr_t)getTempOutDoubleStore(32);               \
          __m128i tmp1 =                                                       \
            (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);      \
          __m256i m1 = _mm256_set_m128i(tmp1, tmp1);                           \
          __m128d tmp2 =                                                       \
            E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);  \
          __m256d m2 = _mm256_set_m128d(tmp2, tmp2);                           \
          align1_double* out = (align1_double*)(base + offset);                \
          func((Ptr_type)out, m1, m2);                                         \
          char str[256];                                                       \
          tostr(&m1, str);                                                     \
          char str2[256];                                                      \
          tostr(&m2, str2);                                                    \
          char str3[256];                                                      \
          tostr(out, numBytesWritten / sizeof(double), str3);                  \
          printf(                                                              \
            "%s(p:align=%d, %s, %s) = %s\n", #func, offset, str, str2, str3);  \
        }

#define void_OutFloatPtr_M256i_M256(                                           \
  func, Ptr_type, numBytesWritten, alignmentBytes)                             \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int j = 0; j < numInterestingInts / 4; ++j)                           \
      for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
        for (int k = 0; k < 4; ++k) {                                          \
          uintptr_t base = (uintptr_t)getTempOutFloatStore(16);                \
          __m128i tmp1 =                                                       \
            (__m128i)E1_Int(interesting_ints, j * 4, numInterestingInts);      \
          __m256i m1 = _mm256_set_m128i(tmp1, tmp1);                           \
          __m128 tmp2 =                                                        \
            E1(interesting_floats, i * 4 + k, numInterestingFloats);           \
          __m256 m2 = _mm256_set_m128(tmp2, tmp2);                             \
          align1_float* out = (align1_float*)(base + offset);                  \
          func((Ptr_type)out, m1, m2);                                         \
          char str[256];                                                       \
          tostr(&m1, str);                                                     \
          char str2[256];                                                      \
          tostr(&m2, str2);                                                    \
          char str3[256];                                                      \
          tostr(out, numBytesWritten / sizeof(float), str3);                   \
          printf(                                                              \
            "%s(p:align=%d, %s, %s) = %s\n", #func, offset, str, str2, str3);  \
        }

#define void_OutFloatPtr_OutFloatPtr_M256(                                     \
  func, Ptr_type, numBytesWritten, alignmentBytes)                             \
  for (int i = 0; i < numInterestingFloats / 4; ++i)                           \
    for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes)   \
      for (int k = 0; k < 4; ++k) {                                            \
        uintptr_t base = (uintptr_t)getTempOutFloatStore(32);                  \
        __m128 tmp = E1(interesting_floats, i * 4 + k, numInterestingFloats);  \
        __m256 m1 = _mm256_set_m128(tmp, tmp);                                 \
        align1_float* out1 = (align1_float*)(base + offset);                   \
        align1_float* out2 = out1 + 4;                                         \
        func((Ptr_type)out1, (Ptr_type)out2, m1);                              \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(out1, numBytesWritten / 2 / sizeof(float), str2);                \
        char str3[256];                                                        \
        tostr(out2, numBytesWritten / 2 / sizeof(float), str3);                \
        printf(                                                                \
          "%s(p:align=%d, %s) = %s,%s\n", #func, offset, str, str2, str3);     \
      }

#define void_OutDoublePtr_OutDoublePtr_M256d(                                  \
  func, Ptr_type, numBytesWritten, alignmentBytes)                             \
  for (int i = 0; i < numInterestingDoubles / 2; ++i)                          \
    for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes)   \
      for (int k = 0; k < 2; ++k) {                                            \
        uintptr_t base = (uintptr_t)getTempOutDoubleStore(32);                 \
        __m128d tmp =                                                          \
          E1_Double(interesting_doubles, i * 2 + k, numInterestingDoubles);    \
        __m256d m1 = _mm256_set_m128d(tmp, tmp);                               \
        align1_double* out1 = (align1_double*)(base + offset);                 \
        align1_double* out2 = out1 + 2;                                        \
        func((Ptr_type)out1, (Ptr_type)out2, m1);                              \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(out1, numBytesWritten / 2 / sizeof(double), str2);               \
        char str3[256];                                                        \
        tostr(out2, numBytesWritten / 2 / sizeof(double), str3);               \
        printf(                                                                \
          "%s(p:align=%d, %s) = %s,%s\n", #func, offset, str, str2, str3);     \
      }

#define void_OutIntPtr_OutIntPtr_M256i(                                        \
  func, Ptr_type, numBytesWritten, alignmentBytes)                             \
  for (int i = 0; i < numInterestingInts / 4; ++i)                             \
    for (int offset = 0; offset < numBytesWritten; offset += alignmentBytes)   \
      for (int k = 0; k < 4; ++k) {                                            \
        uintptr_t base = (uintptr_t)getTempOutIntStore(32);                    \
        __m128i tmp =                                                          \
          (__m128i)E1_Int(interesting_ints, i * 4 + k, numInterestingInts);    \
        __m256i m1 = _mm256_set_m128i(tmp, tmp);                               \
        align1_int* out1 = (align1_int*)(base + offset);                       \
        align1_int* out2 = out1 + 4;                                           \
        func((Ptr_type)out1, (Ptr_type)out2, m1);                              \
        char str[256];                                                         \
        tostr(&m1, str);                                                       \
        char str2[256];                                                        \
        tostr(                                                                 \
          out1, (numBytesWritten + sizeof(int) - 1) / 2 / sizeof(int), str2);  \
        char str3[256];                                                        \
        tostr(                                                                 \
          out2, (numBytesWritten + sizeof(int) - 1) / 2 / sizeof(int), str3);  \
        printf(                                                                \
          "%s(p:align=%d, %s) = %s,%s\n", #func, offset, str, str2, str3);     \
      }

#define Ret_Double2(Ret_type, func, inc)                                       \
  for (int i = 0; i + 2 <= numInterestingDoubles; i += inc) {                  \
    double* ptr = interesting_doubles + i;                                     \
    Ret_type ret = func(ptr[0], ptr[1]);                                       \
    char str[256];                                                             \
    tostr(ptr, 2, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Double4(Ret_type, func, inc)                                       \
  for (int i = 0; i + 4 <= numInterestingDoubles; i += inc) {                  \
    double* ptr = interesting_doubles + i;                                     \
    Ret_type ret = func(ptr[0], ptr[1], ptr[2], ptr[3]);                       \
    char str[256];                                                             \
    tostr(ptr, 4, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Float8(Ret_type, func, inc)                                        \
  for (int i = 0; i + 8 <= numInterestingFloats; i += inc) {                   \
    float* ptr = interesting_floats + i;                                       \
    Ret_type ret =                                                             \
      func(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);    \
    char str[256];                                                             \
    tostr(ptr, 8, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Int8(Ret_type, func, inc)                                          \
  for (int i = 0; i + 8 <= numInterestingInts; i += inc) {                     \
    int* ptr = (int*)interesting_ints + i;                                     \
    Ret_type ret =                                                             \
      func(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);    \
    char str[256];                                                             \
    tostr(ptr, 8, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Short16(Ret_type, func, inc)                                       \
  for (int i = 0; i + 16 <= numInterestingInts * 2; i += inc) {                \
    short* ptr = ((short*)interesting_ints) + i;                               \
    Ret_type ret = func(ptr[0],                                                \
                        ptr[1],                                                \
                        ptr[2],                                                \
                        ptr[3],                                                \
                        ptr[4],                                                \
                        ptr[5],                                                \
                        ptr[6],                                                \
                        ptr[7],                                                \
                        ptr[8],                                                \
                        ptr[9],                                                \
                        ptr[10],                                               \
                        ptr[11],                                               \
                        ptr[12],                                               \
                        ptr[13],                                               \
                        ptr[14],                                               \
                        ptr[15]);                                              \
    char str[256];                                                             \
    tostr((int*)ptr, 8, str);                                                  \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Char32(Ret_type, func, inc)                                        \
  for (int i = 0; i + 32 <= numInterestingInts * 4; i += inc) {                \
    char* ptr = ((char*)interesting_ints) + i;                                 \
    Ret_type ret = func(ptr[0],                                                \
                        ptr[1],                                                \
                        ptr[2],                                                \
                        ptr[3],                                                \
                        ptr[4],                                                \
                        ptr[5],                                                \
                        ptr[6],                                                \
                        ptr[7],                                                \
                        ptr[8],                                                \
                        ptr[9],                                                \
                        ptr[10],                                               \
                        ptr[11],                                               \
                        ptr[12],                                               \
                        ptr[13],                                               \
                        ptr[14],                                               \
                        ptr[15],                                               \
                        ptr[16],                                               \
                        ptr[17],                                               \
                        ptr[18],                                               \
                        ptr[19],                                               \
                        ptr[20],                                               \
                        ptr[21],                                               \
                        ptr[22],                                               \
                        ptr[23],                                               \
                        ptr[24],                                               \
                        ptr[25],                                               \
                        ptr[26],                                               \
                        ptr[27],                                               \
                        ptr[28],                                               \
                        ptr[29],                                               \
                        ptr[30],                                               \
                        ptr[31]);                                              \
    char str[256];                                                             \
    tostr((int*)ptr, 8, str);                                                  \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Longlong4(Ret_type, func, inc)                                     \
  for (int i = 0; i + 4 <= numInterestingInts / 2; i += inc) {                 \
    long long* ptr = ((long long*)interesting_ints) + i;                       \
    Ret_type ret = func(ptr[0], ptr[1], ptr[2], ptr[3]);                       \
    char str[256];                                                             \
    tostr((int*)ptr, 8, str);                                                  \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Double(Ret_type, func, inc)                                        \
  for (int i = 0; i + 1 <= numInterestingDoubles; i += inc) {                  \
    double* ptr = interesting_doubles + i;                                     \
    Ret_type ret = func(*ptr);                                                 \
    char str[256];                                                             \
    tostr(ptr, 1, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#define Ret_Int(Ret_type, func, inc)                                           \
  for (int i = 0; i + 1 <= numInterestingInts; i += inc) {                     \
    int* ptr = ((int*)interesting_ints) + i;                                   \
    Ret_type ret = func(*ptr);                                                 \
    char str[256];                                                             \
    tostr(ptr, 1, str);                                                        \
    char str2[256];                                                            \
    tostr(&ret, str2);                                                         \
    printf("%s(%s) = %s\n", #func, str, str2);                                 \
  }

#endif
