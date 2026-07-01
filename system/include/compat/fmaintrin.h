/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * FMA intrinsics implementation for Emscripten.
 * Emulates x86 FMA (Fused Multiply-Add) operations using Wasm SIMD.
 *
 * With -mrelaxed-simd: uses Wasm relaxed SIMD FMA, which the host engine may
 * lower to a hardware fused multiply-add (single rounding step) where
 * available, e.g. on x86/ARM with FMA support. The relaxed SIMD spec leaves
 * fusion implementation-defined, so on hosts without hardware FMA the result
 * may instead be a separate multiply and add (two rounding steps).
 * With -msimd128 only: emulates FMA with separate multiply and add/sub
 * (two rounding steps).
 */

#ifndef __emscripten_immintrin_h__
#error "Never use <fmaintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __emscripten_fmaintrin_h__
#define __emscripten_fmaintrin_h__

#ifndef __FMA__
#error "FMA instruction set not enabled"
#endif

#ifdef __wasm_relaxed_simd__
#include <wasm_simd128.h>
#endif

/* ============================================================
 * 128-bit packed float (ps) — 4x float
 * ============================================================ */

/* a * b + c */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fmadd_ps(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128)wasm_f32x4_relaxed_madd(
    (__f32x4)__A, (__f32x4)__B, (__f32x4)__C);
#else
  return _mm_add_ps(_mm_mul_ps(__A, __B), __C);
#endif
}

/* a * b - c */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fmsub_ps(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128)wasm_f32x4_relaxed_madd(
    (__f32x4)__A, (__f32x4)__B, (__f32x4)_mm_xor_ps(__C, _mm_set1_ps(-0.0f)));
#else
  return _mm_sub_ps(_mm_mul_ps(__A, __B), __C);
#endif
}

/* -(a * b) + c */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fnmadd_ps(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128)wasm_f32x4_relaxed_nmadd(
    (__f32x4)__A, (__f32x4)__B, (__f32x4)__C);
#else
  return _mm_sub_ps(__C, _mm_mul_ps(__A, __B));
#endif
}

/* -(a * b) - c */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fnmsub_ps(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128)wasm_f32x4_relaxed_nmadd(
    (__f32x4)__A, (__f32x4)__B, (__f32x4)_mm_xor_ps(__C, _mm_set1_ps(-0.0f)));
#else
  __m128 neg_ab = _mm_sub_ps(_mm_setzero_ps(), _mm_mul_ps(__A, __B));
  return _mm_sub_ps(neg_ab, __C);
#endif
}

/* even elements: a*b - c, odd elements: a*b + c */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fmaddsub_ps(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  __m128 neg_c =
    _mm_xor_ps(__C, (__m128)_mm_set_epi32(0, 0x80000000, 0, 0x80000000));
  return (__m128)wasm_f32x4_relaxed_madd(
    (__f32x4)__A, (__f32x4)__B, (__f32x4)neg_c);
#else
  __m128 add = _mm_add_ps(_mm_mul_ps(__A, __B), __C);
  __m128 sub = _mm_sub_ps(_mm_mul_ps(__A, __B), __C);
  return _mm_blend_ps(sub, add, 0xA); /* 0xA = 1010b: elements 1,3 from add */
#endif
}

/* even elements: a*b + c, odd elements: a*b - c */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fmsubadd_ps(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  __m128 neg_c =
    _mm_xor_ps(__C, (__m128)_mm_set_epi32(0x80000000, 0, 0x80000000, 0));
  return (__m128)wasm_f32x4_relaxed_madd(
    (__f32x4)__A, (__f32x4)__B, (__f32x4)neg_c);
#else
  __m128 add = _mm_add_ps(_mm_mul_ps(__A, __B), __C);
  __m128 sub = _mm_sub_ps(_mm_mul_ps(__A, __B), __C);
  return _mm_blend_ps(add, sub, 0xA); /* 0xA = 1010b: elements 1,3 from sub */
#endif
}

/* ============================================================
 * 128-bit packed double (pd) — 2x double
 * ============================================================ */

/* a * b + c */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fmadd_pd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128d)wasm_f64x2_relaxed_madd(
    (__f64x2)__A, (__f64x2)__B, (__f64x2)__C);
#else
  return _mm_add_pd(_mm_mul_pd(__A, __B), __C);
#endif
}

/* a * b - c */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fmsub_pd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128d)wasm_f64x2_relaxed_madd(
    (__f64x2)__A, (__f64x2)__B, (__f64x2)_mm_xor_pd(__C, _mm_set1_pd(-0.0)));
#else
  return _mm_sub_pd(_mm_mul_pd(__A, __B), __C);
#endif
}

/* -(a * b) + c */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fnmadd_pd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128d)wasm_f64x2_relaxed_nmadd(
    (__f64x2)__A, (__f64x2)__B, (__f64x2)__C);
#else
  return _mm_sub_pd(__C, _mm_mul_pd(__A, __B));
#endif
}

/* -(a * b) - c */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fnmsub_pd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  return (__m128d)wasm_f64x2_relaxed_nmadd(
    (__f64x2)__A, (__f64x2)__B, (__f64x2)_mm_xor_pd(__C, _mm_set1_pd(-0.0)));
#else
  __m128d neg_ab = _mm_sub_pd(_mm_setzero_pd(), _mm_mul_pd(__A, __B));
  return _mm_sub_pd(neg_ab, __C);
#endif
}

/* even elements: a*b - c, odd elements: a*b + c */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fmaddsub_pd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  __m128d neg_c =
    _mm_xor_pd(__C, (__m128d)_mm_set_epi64x(0, 0x8000000000000000LL));
  return (__m128d)wasm_f64x2_relaxed_madd(
    (__f64x2)__A, (__f64x2)__B, (__f64x2)neg_c);
#else
  __m128d add = _mm_add_pd(_mm_mul_pd(__A, __B), __C);
  __m128d sub = _mm_sub_pd(_mm_mul_pd(__A, __B), __C);
  return _mm_blend_pd(sub, add, 0x2); /* 0x2 = 10b: element 1 from add */
#endif
}

/* even elements: a*b + c, odd elements: a*b - c */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fmsubadd_pd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  __m128d neg_c =
    _mm_xor_pd(__C, (__m128d)_mm_set_epi64x(0x8000000000000000LL, 0));
  return (__m128d)wasm_f64x2_relaxed_madd(
    (__f64x2)__A, (__f64x2)__B, (__f64x2)neg_c);
#else
  __m128d add = _mm_add_pd(_mm_mul_pd(__A, __B), __C);
  __m128d sub = _mm_sub_pd(_mm_mul_pd(__A, __B), __C);
  return _mm_blend_pd(add, sub, 0x2); /* 0x2 = 10b: element 1 from sub */
#endif
}

/* ============================================================
 * Scalar float (ss) — lowest element only, upper from first operand
 * ============================================================ */

/* a[0] * b[0] + c[0], a[1..3] pass through */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fmadd_ss(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  return _mm_move_ss(
    __A,
    (__m128)wasm_f32x4_relaxed_madd((__f32x4)__A, (__f32x4)__B, (__f32x4)__C));
#else
  return _mm_move_ss(__A, _mm_add_ss(_mm_mul_ss(__A, __B), __C));
#endif
}

/* a[0] * b[0] - c[0], a[1..3] pass through */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fmsub_ss(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  __m128 neg_c = _mm_xor_ps(__C, _mm_set1_ps(-0.0f));
  return _mm_move_ss(__A,
                     (__m128)wasm_f32x4_relaxed_madd(
                       (__f32x4)__A, (__f32x4)__B, (__f32x4)neg_c));
#else
  return _mm_move_ss(__A, _mm_sub_ss(_mm_mul_ss(__A, __B), __C));
#endif
}

/* -(a[0] * b[0]) + c[0], a[1..3] pass through */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fnmadd_ss(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  return _mm_move_ss(
    __A,
    (__m128)wasm_f32x4_relaxed_nmadd((__f32x4)__A, (__f32x4)__B, (__f32x4)__C));
#else
  return _mm_move_ss(__A, _mm_sub_ss(__C, _mm_mul_ss(__A, __B)));
#endif
}

/* -(a[0] * b[0]) - c[0], a[1..3] pass through */
static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_fnmsub_ss(__m128 __A, __m128 __B, __m128 __C) {
#ifdef __wasm_relaxed_simd__
  __m128 neg_c = _mm_xor_ps(__C, _mm_set1_ps(-0.0f));
  return _mm_move_ss(__A,
                     (__m128)wasm_f32x4_relaxed_nmadd(
                       (__f32x4)__A, (__f32x4)__B, (__f32x4)neg_c));
#else
  __m128 neg_ab = _mm_sub_ss(_mm_setzero_ps(), _mm_mul_ss(__A, __B));
  return _mm_move_ss(__A, _mm_sub_ss(neg_ab, __C));
#endif
}

/* ============================================================
 * Scalar double (sd) — lowest element only, upper from first operand
 * ============================================================ */

/* a[0] * b[0] + c[0], a[1] pass through */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fmadd_sd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  return _mm_move_sd(
    __A,
    (__m128d)wasm_f64x2_relaxed_madd((__f64x2)__A, (__f64x2)__B, (__f64x2)__C));
#else
  return _mm_move_sd(__A, _mm_add_sd(_mm_mul_sd(__A, __B), __C));
#endif
}

/* a[0] * b[0] - c[0], a[1] pass through */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fmsub_sd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  __m128d neg_c = _mm_xor_pd(__C, _mm_set1_pd(-0.0));
  return _mm_move_sd(__A,
                     (__m128d)wasm_f64x2_relaxed_madd(
                       (__f64x2)__A, (__f64x2)__B, (__f64x2)neg_c));
#else
  return _mm_move_sd(__A, _mm_sub_sd(_mm_mul_sd(__A, __B), __C));
#endif
}

/* -(a[0] * b[0]) + c[0], a[1] pass through */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fnmadd_sd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  return _mm_move_sd(__A,
                     (__m128d)wasm_f64x2_relaxed_nmadd(
                       (__f64x2)__A, (__f64x2)__B, (__f64x2)__C));
#else
  return _mm_move_sd(__A, _mm_sub_sd(__C, _mm_mul_sd(__A, __B)));
#endif
}

/* -(a[0] * b[0]) - c[0], a[1] pass through */
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_fnmsub_sd(__m128d __A, __m128d __B, __m128d __C) {
#ifdef __wasm_relaxed_simd__
  __m128d neg_c = _mm_xor_pd(__C, _mm_set1_pd(-0.0));
  return _mm_move_sd(__A,
                     (__m128d)wasm_f64x2_relaxed_nmadd(
                       (__f64x2)__A, (__f64x2)__B, (__f64x2)neg_c));
#else
  __m128d neg_ab = _mm_sub_sd(_mm_setzero_pd(), _mm_mul_sd(__A, __B));
  return _mm_move_sd(__A, _mm_sub_sd(neg_ab, __C));
#endif
}

#ifdef __AVX__
/* ============================================================
 * 256-bit packed float (ps) — 8x float
 * ============================================================ */

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_fmadd_ps(__m256 __A, __m256 __B, __m256 __C) {
  __m256_internal a = __m256_to_internal(__A);
  __m256_internal b = __m256_to_internal(__B);
  __m256_internal c = __m256_to_internal(__C);
  __m256_internal ret;
  ret.v0 = _mm_fmadd_ps(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmadd_ps(a.v1, b.v1, c.v1);
  return __m256_from_internal(ret);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_fmsub_ps(__m256 __A, __m256 __B, __m256 __C) {
  __m256_internal a = __m256_to_internal(__A);
  __m256_internal b = __m256_to_internal(__B);
  __m256_internal c = __m256_to_internal(__C);
  __m256_internal ret;
  ret.v0 = _mm_fmsub_ps(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmsub_ps(a.v1, b.v1, c.v1);
  return __m256_from_internal(ret);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_fnmadd_ps(__m256 __A, __m256 __B, __m256 __C) {
  __m256_internal a = __m256_to_internal(__A);
  __m256_internal b = __m256_to_internal(__B);
  __m256_internal c = __m256_to_internal(__C);
  __m256_internal ret;
  ret.v0 = _mm_fnmadd_ps(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fnmadd_ps(a.v1, b.v1, c.v1);
  return __m256_from_internal(ret);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_fnmsub_ps(__m256 __A, __m256 __B, __m256 __C) {
  __m256_internal a = __m256_to_internal(__A);
  __m256_internal b = __m256_to_internal(__B);
  __m256_internal c = __m256_to_internal(__C);
  __m256_internal ret;
  ret.v0 = _mm_fnmsub_ps(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fnmsub_ps(a.v1, b.v1, c.v1);
  return __m256_from_internal(ret);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_fmaddsub_ps(__m256 __A, __m256 __B, __m256 __C) {
  __m256_internal a = __m256_to_internal(__A);
  __m256_internal b = __m256_to_internal(__B);
  __m256_internal c = __m256_to_internal(__C);
  __m256_internal ret;
  ret.v0 = _mm_fmaddsub_ps(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmaddsub_ps(a.v1, b.v1, c.v1);
  return __m256_from_internal(ret);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_fmsubadd_ps(__m256 __A, __m256 __B, __m256 __C) {
  __m256_internal a = __m256_to_internal(__A);
  __m256_internal b = __m256_to_internal(__B);
  __m256_internal c = __m256_to_internal(__C);
  __m256_internal ret;
  ret.v0 = _mm_fmsubadd_ps(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmsubadd_ps(a.v1, b.v1, c.v1);
  return __m256_from_internal(ret);
}

/* ============================================================
 * 256-bit packed double (pd) — 4x double
 * ============================================================ */

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_fmadd_pd(__m256d __A, __m256d __B, __m256d __C) {
  __m256d_internal a = __m256d_to_internal(__A);
  __m256d_internal b = __m256d_to_internal(__B);
  __m256d_internal c = __m256d_to_internal(__C);
  __m256d_internal ret;
  ret.v0 = _mm_fmadd_pd(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmadd_pd(a.v1, b.v1, c.v1);
  return __m256d_from_internal(ret);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_fmsub_pd(__m256d __A, __m256d __B, __m256d __C) {
  __m256d_internal a = __m256d_to_internal(__A);
  __m256d_internal b = __m256d_to_internal(__B);
  __m256d_internal c = __m256d_to_internal(__C);
  __m256d_internal ret;
  ret.v0 = _mm_fmsub_pd(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmsub_pd(a.v1, b.v1, c.v1);
  return __m256d_from_internal(ret);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_fnmadd_pd(__m256d __A, __m256d __B, __m256d __C) {
  __m256d_internal a = __m256d_to_internal(__A);
  __m256d_internal b = __m256d_to_internal(__B);
  __m256d_internal c = __m256d_to_internal(__C);
  __m256d_internal ret;
  ret.v0 = _mm_fnmadd_pd(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fnmadd_pd(a.v1, b.v1, c.v1);
  return __m256d_from_internal(ret);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_fnmsub_pd(__m256d __A, __m256d __B, __m256d __C) {
  __m256d_internal a = __m256d_to_internal(__A);
  __m256d_internal b = __m256d_to_internal(__B);
  __m256d_internal c = __m256d_to_internal(__C);
  __m256d_internal ret;
  ret.v0 = _mm_fnmsub_pd(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fnmsub_pd(a.v1, b.v1, c.v1);
  return __m256d_from_internal(ret);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_fmaddsub_pd(__m256d __A, __m256d __B, __m256d __C) {
  __m256d_internal a = __m256d_to_internal(__A);
  __m256d_internal b = __m256d_to_internal(__B);
  __m256d_internal c = __m256d_to_internal(__C);
  __m256d_internal ret;
  ret.v0 = _mm_fmaddsub_pd(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmaddsub_pd(a.v1, b.v1, c.v1);
  return __m256d_from_internal(ret);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_fmsubadd_pd(__m256d __A, __m256d __B, __m256d __C) {
  __m256d_internal a = __m256d_to_internal(__A);
  __m256d_internal b = __m256d_to_internal(__B);
  __m256d_internal c = __m256d_to_internal(__C);
  __m256d_internal ret;
  ret.v0 = _mm_fmsubadd_pd(a.v0, b.v0, c.v0);
  ret.v1 = _mm_fmsubadd_pd(a.v1, b.v1, c.v1);
  return __m256d_from_internal(ret);
}

#endif /* __AVX__ */

#endif /* __emscripten_fmaintrin_h__ */
