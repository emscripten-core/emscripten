/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_smmintrin_h__
#define __emscripten_smmintrin_h__

#ifndef __SSE4_1__
#error "SSE4.1 instruction set not enabled"
#endif

#include <tmmintrin.h>
#include <math.h> // For rint and rintf

#define _mm_blend_epi16(__a, __b, __imm8) __extension__ ({     \
  (__m128i)__builtin_shufflevector((__i16x8)(__m128i)(__a),    \
                                   (__i16x8)(__m128i)(__b),    \
                                   (((__imm8) & 1) ?  8 : 0),  \
                                   (((__imm8) & 2) ?  9 : 1),  \
                                   (((__imm8) & 4) ? 10 : 2),  \
                                   (((__imm8) & 8) ? 11 : 3),  \
                                   (((__imm8) & 16) ? 12 : 4), \
                                   (((__imm8) & 32) ? 13 : 5), \
                                   (((__imm8) & 64) ? 14 : 6), \
                                   (((__imm8) & 128) ? 15 : 7)); })

#define _mm_blend_pd(__a, __b, __imm8) __extension__ ({         \
  (__m128d)__builtin_shufflevector((__f64x2)(__m128d)(__a),     \
                                   (__f64x2)(__m128d)(__b),     \
                                   (((__imm8) & 0x01) ? 2 : 0), \
                                   (((__imm8) & 0x02) ? 3 : 1)); })

#define _mm_blend_ps(__a, __b, __imm8) __extension__ ({ \
  (__m128)__builtin_shufflevector((__f32x4)(__m128)(__a), (__f32x4)(__m128)(__b), \
                                  (((__imm8) & 0x01) ? 4 : 0), \
                                  (((__imm8) & 0x02) ? 5 : 1), \
                                  (((__imm8) & 0x04) ? 6 : 2), \
                                  (((__imm8) & 0x08) ? 7 : 3)); })

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_blendv_epi8(__m128i __a, __m128i __b, __m128i __mask)
{
  v128_t __M = wasm_i8x16_shr((v128_t)__mask, 7);
  return (__m128i)wasm_v128_bitselect((v128_t)__b, (v128_t)__a, __M);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_blendv_pd(__m128d __a, __m128d __b, __m128d __mask)
{
  v128_t __M = wasm_i64x2_shr((v128_t)__mask, 63);
  return (__m128d)wasm_v128_bitselect((v128_t)__b, (v128_t)__a, __M);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_blendv_ps(__m128 __a, __m128 __b, __m128 __mask)
{
  v128_t __M = wasm_i32x4_shr((v128_t)__mask, 31);
  return (__m128)wasm_v128_bitselect((v128_t)__b, (v128_t)__a, __M);
}

#define _MM_FROUND_TO_NEAREST_INT    0x00
#define _MM_FROUND_TO_NEG_INF        0x01
#define _MM_FROUND_TO_POS_INF        0x02
#define _MM_FROUND_TO_ZERO           0x03
#define _MM_FROUND_CUR_DIRECTION     0x04

#define _MM_FROUND_RAISE_EXC         0x00
#define _MM_FROUND_NO_EXC            0x08

#define _MM_FROUND_NINT      (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_NEAREST_INT)
#define _MM_FROUND_FLOOR     (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_NEG_INF)
#define _MM_FROUND_CEIL      (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_POS_INF)
#define _MM_FROUND_TRUNC     (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_ZERO)
#define _MM_FROUND_RINT      (_MM_FROUND_RAISE_EXC | _MM_FROUND_CUR_DIRECTION)
#define _MM_FROUND_NEARBYINT (_MM_FROUND_NO_EXC | _MM_FROUND_CUR_DIRECTION)

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_ceil_pd(__m128d __a)
{
  return (__m128d)wasm_f64x2_make(__builtin_ceil(wasm_f64x2_extract_lane(__a, 0)),
                                  __builtin_ceil(wasm_f64x2_extract_lane(__a, 1)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_ceil_ps(__m128 __a)
{
  return (__m128)wasm_f32x4_make(__builtin_ceilf(wasm_f32x4_extract_lane(__a, 0)),
                                 __builtin_ceilf(wasm_f32x4_extract_lane(__a, 1)),
                                 __builtin_ceilf(wasm_f32x4_extract_lane(__a, 2)),
                                 __builtin_ceilf(wasm_f32x4_extract_lane(__a, 3)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_ceil_ss(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_replace_lane(__a, 0, __builtin_ceilf(wasm_f32x4_extract_lane(__b, 0)));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_ceil_sd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_replace_lane(__a, 0, __builtin_ceil(wasm_f64x2_extract_lane(__b, 0)));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_floor_pd(__m128d __a)
{
  return (__m128d)wasm_f64x2_make(__builtin_floor(wasm_f64x2_extract_lane(__a, 0)),
                                  __builtin_floor(wasm_f64x2_extract_lane(__a, 1)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_floor_ps(__m128 __a)
{
  return (__m128)wasm_f32x4_make(__builtin_floorf(wasm_f32x4_extract_lane(__a, 0)),
                                 __builtin_floorf(wasm_f32x4_extract_lane(__a, 1)),
                                 __builtin_floorf(wasm_f32x4_extract_lane(__a, 2)),
                                 __builtin_floorf(wasm_f32x4_extract_lane(__a, 3)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_floor_ss(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_replace_lane(__a, 0, __builtin_floorf(wasm_f32x4_extract_lane(__b, 0)));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_floor_sd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_replace_lane(__a, 0, __builtin_floor(wasm_f64x2_extract_lane(__b, 0)));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_round_pd(__m128d __a, int __rounding)
{
  switch(__rounding & 7)
  {
    case _MM_FROUND_TO_NEG_INF: return _mm_floor_pd(__a);
    case _MM_FROUND_TO_POS_INF: return _mm_ceil_pd(__a);
    case _MM_FROUND_TO_ZERO:
      return (__m128d)wasm_f64x2_make(__builtin_trunc(wasm_f64x2_extract_lane(__a, 0)),
                                      __builtin_trunc(wasm_f64x2_extract_lane(__a, 1)));
    default:
      // _MM_FROUND_TO_NEAREST_INT and _MM_FROUND_CUR_DIRECTION (which is always nearest in Wasm SIMD)
      // SSE implements "Banker's rounding", where even half-ways, e.g. 2.5 are rounded down,
      // and odd numbers e.g. 3.5 are rounded up.
      return (__m128d)wasm_f64x2_make(rint(wasm_f64x2_extract_lane(__a, 0)),
                                      rint(wasm_f64x2_extract_lane(__a, 1)));
  }
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_round_ps(__m128 __a, int __rounding)
{
  switch(__rounding & 7)
  {
    case _MM_FROUND_TO_NEG_INF: return _mm_floor_ps(__a);
    case _MM_FROUND_TO_POS_INF: return _mm_ceil_ps(__a);
    case _MM_FROUND_TO_ZERO:
      return (__m128)wasm_f32x4_make(__builtin_truncf(wasm_f32x4_extract_lane(__a, 0)),
                                     __builtin_truncf(wasm_f32x4_extract_lane(__a, 1)),
                                     __builtin_truncf(wasm_f32x4_extract_lane(__a, 2)),
                                     __builtin_truncf(wasm_f32x4_extract_lane(__a, 3)));
    default:
      // _MM_FROUND_TO_NEAREST_INT and _MM_FROUND_CUR_DIRECTION (which is always nearest in Wasm SIMD)
      // SSE implements "Banker's rounding", where even half-ways, e.g. 2.5 are rounded down,
      // and odd numbers e.g. 3.5 are rounded up.
      return (__m128)wasm_f32x4_make(rintf(wasm_f32x4_extract_lane(__a, 0)),
                                     rintf(wasm_f32x4_extract_lane(__a, 1)),
                                     rintf(wasm_f32x4_extract_lane(__a, 2)),
                                     rintf(wasm_f32x4_extract_lane(__a, 3)));
  }
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_round_ss(__m128 __a, __m128 __b, int __rounding)
{
  switch(__rounding & 7)
  {
    case _MM_FROUND_TO_NEG_INF: return _mm_floor_ss(__a, __b);
    case _MM_FROUND_TO_POS_INF: return _mm_ceil_ss(__a, __b);
    case _MM_FROUND_TO_ZERO:
      return (__m128)wasm_f32x4_replace_lane(__a, 0, __builtin_truncf(wasm_f32x4_extract_lane(__b, 0)));
    default:
      // _MM_FROUND_TO_NEAREST_INT and _MM_FROUND_CUR_DIRECTION (which is always nearest in Wasm SIMD)
      // SSE implements "Banker's rounding", where even half-ways, e.g. 2.5 are rounded down,
      // and odd numbers e.g. 3.5 are rounded up.
      return (__m128)wasm_f32x4_replace_lane(__a, 0, rintf(wasm_f32x4_extract_lane(__b, 0)));
  }
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_round_sd(__m128d __a, __m128d __b, int __rounding)
{
  switch(__rounding & 7)
  {
    case _MM_FROUND_TO_NEG_INF: return _mm_floor_sd(__a, __b);
    case _MM_FROUND_TO_POS_INF: return _mm_ceil_sd(__a, __b);
    case _MM_FROUND_TO_ZERO:
      return (__m128d)wasm_f64x2_replace_lane(__a, 0, __builtin_trunc(wasm_f64x2_extract_lane(__b, 0)));
    default:
      // _MM_FROUND_TO_NEAREST_INT and _MM_FROUND_CUR_DIRECTION (which is always nearest in Wasm SIMD)
      // SSE implements "Banker's rounding", where even half-ways, e.g. 2.5 are rounded down,
      // and odd numbers e.g. 3.5 are rounded up.
      return (__m128d)wasm_f64x2_replace_lane(__a, 0, rint(wasm_f64x2_extract_lane(__b, 0)));
  }
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mullo_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_mul(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i64x2_make(
      wasm_i32x4_extract_lane(__a, 0) * (long long)wasm_i32x4_extract_lane(__b, 0),
      wasm_i32x4_extract_lane(__a, 2) * (long long)wasm_i32x4_extract_lane(__b, 2));
}

#define _mm_dp_ps(__a, __b, __imm8) __extension__ ({ \
        __m128 __tmp = _mm_mul_ps(__a, __b); \
        __m128 __zero = _mm_setzero_ps(); \
        __tmp = _mm_blend_ps(__zero, __tmp, __imm8 >> 4); \
        __m128 __sum = _mm_add_ps(__tmp, _mm_shuffle_ps(__tmp, __tmp, _MM_SHUFFLE(2, 3, 0, 1))); \
        __sum = _mm_add_ps(__sum, _mm_shuffle_ps(__sum, __sum, _MM_SHUFFLE(1, 0, 3, 2))); \
        _mm_blend_ps(__zero, __sum, __imm8); })

#define _mm_dp_pd(__a, __b, __imm8) __extension__ ({ \
        __m128d __tmp = _mm_mul_pd(__a, __b); \
        __m128d __zero = _mm_setzero_pd(); \
        __tmp = _mm_blend_pd(__zero, __tmp, __imm8 >> 4); \
        __m128d __sum = _mm_add_pd(__tmp, _mm_shuffle_pd(__tmp, __tmp, _MM_SHUFFLE2(0, 1))); \
        _mm_blend_pd(__zero, __sum, __imm8); })

#define _mm_stream_load_si128 _mm_load_si128

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_min(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_max(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u16x8_min(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u16x8_max(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_min(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_max(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u32x4_min(__a, __b);
}

static __inline__  __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u32x4_max(__a, __b);
}

#define _mm_insert_ps(__a, __b, __imm8) __extension__ ({       \
        _Static_assert(__builtin_constant_p(__imm8), "Expected constant"); \
        __m128 __tmp = __builtin_shufflevector((__f32x4)__a, (__f32x4)__b, \
                            ((((__imm8) >> 4) & 3) == 0) ? ((((__imm8) >> 6) & 3) + 4) : 0, \
                            ((((__imm8) >> 4) & 3) == 1) ? ((((__imm8) >> 6) & 3) + 4) : 1, \
                            ((((__imm8) >> 4) & 3) == 2) ? ((((__imm8) >> 6) & 3) + 4) : 2, \
                            ((((__imm8) >> 4) & 3) == 3) ? ((((__imm8) >> 6) & 3) + 4) : 3); \
        (__m128)__builtin_shufflevector(__tmp, _mm_setzero_ps(), \
                                        (((__imm8) & 1) ? 4 : 0), \
                                        (((__imm8) & 2) ? 5 : 1), \
                                        (((__imm8) & 4) ? 6 : 2), \
                                        (((__imm8) & 8) ? 7 : 3)); })

#define _mm_extract_ps(__a, __imm8) __extension__ ({       \
                                       wasm_i32x4_extract_lane((__a), (__imm8) & 3); })

#define _MM_EXTRACT_FLOAT(D, X, N) (__extension__ ({ __f32x4 __a = (__f32x4)(X); \
                                                    (D) = __a[N]; }))

#define _MM_MK_INSERTPS_NDX(X, Y, Z) (((X) << 6) | ((Y) << 4) | (Z))

#define _MM_PICK_OUT_PS(X, N) _mm_insert_ps(_mm_setzero_ps(), (X),   \
                                             _MM_MK_INSERTPS_NDX((N), 0, 0x0e))

#define _mm_insert_epi8(__a, __i, __imm8) __extension__ ({    \
                                     (__m128i)wasm_i8x16_replace_lane((__a), (__imm8) & 15, (__i)); })

#define _mm_insert_epi32(__a, __i, __imm8) __extension__ ({    \
                                     (__m128i)wasm_i32x4_replace_lane((__a), (__imm8) & 3, (__i)); })

#define _mm_insert_epi64(__a, __i, __imm8) __extension__ ({    \
                                     (__m128i)wasm_i64x2_replace_lane((__a), (__imm8) & 1, (__i)); })

#define _mm_extract_epi8(__a, __imm8) __extension__ ({       \
                                       wasm_u8x16_extract_lane((__a), (__imm8) & 15); })

#define _mm_extract_epi32(__a, __imm8) __extension__ ({       \
                                       wasm_i32x4_extract_lane((__a), (__imm8) & 3); })

#define _mm_extract_epi64(__a, __imm8) __extension__ ({       \
                                       wasm_i64x2_extract_lane((__a), (__imm8) & 1); })

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testz_si128(__m128i __a, __m128i __b)
{
  v128_t __m = wasm_v128_and(__a, __b);
  return (wasm_i64x2_extract_lane(__m, 0) | wasm_i64x2_extract_lane(__m, 1)) == 0;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testc_si128(__m128i __a, __m128i __b)
{
  v128_t __m = wasm_v128_andnot(__b, __a);
  return (wasm_i64x2_extract_lane(__m, 0) | wasm_i64x2_extract_lane(__m, 1)) == 0;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testnzc_si128(__m128i __a, __m128i __b)
{
  v128_t __m1 = wasm_v128_and(__a, __b);
  v128_t __m2 = wasm_v128_andnot(__b, __a);
  return (wasm_i64x2_extract_lane(__m1, 0) | wasm_i64x2_extract_lane(__m1, 1))
      && (wasm_i64x2_extract_lane(__m2, 0) | wasm_i64x2_extract_lane(__m2, 1));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_test_all_ones(__m128i __a)
{
  return (wasm_i64x2_extract_lane(__a, 0) & wasm_i64x2_extract_lane(__a, 1)) == 0xFFFFFFFFFFFFFFFFull;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_test_all_zeros(__m128i __a, __m128i __mask)
{
  v128_t __m = wasm_v128_and(__a, __mask);
  return (wasm_i64x2_extract_lane(__m, 0) | wasm_i64x2_extract_lane(__m, 1)) == 0;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_test_mix_ones_zeros(__m128i __a, __m128i __mask)
{
  v128_t __m = wasm_v128_and(__a, __mask);
  long long __c0 = wasm_i64x2_extract_lane(__m, 0);
  long long __c1 = wasm_i64x2_extract_lane(__m, 1);
  long long __ones = __c0 | __c1;
  long long __zeros = ~(__c0 & __c1);
  return __ones && __zeros;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi64(__m128i __a, __m128i __b)
{
  const __m128i __mask = _mm_cmpeq_epi32(__a, __b);
  return _mm_and_si128(__mask, _mm_shuffle_epi32(__mask, _MM_SHUFFLE(2, 3, 0, 1)));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi8_epi16(__m128i __a)
{
  return (__m128i)wasm_i16x8_widen_low_i8x16((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi8_epi32(__m128i __a)
{
  return (__m128i)wasm_i32x4_widen_low_i16x8(wasm_i16x8_widen_low_i8x16((v128_t)__a));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi8_epi64(__m128i __a)
{
  const __m128i __exta = _mm_cvtepi8_epi32(__a);
  const __m128i __sign = _mm_cmpgt_epi32(_mm_setzero_si128(), __exta);
  return _mm_unpacklo_epi32(__exta, __sign);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi16_epi32(__m128i __a)
{
  return (__m128i)wasm_i32x4_widen_low_i16x8((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi16_epi64(__m128i __a)
{
  const __m128i __exta = _mm_cvtepi16_epi32(__a);
  const __m128i __sign = _mm_cmpgt_epi32(_mm_setzero_si128(), __exta);
  return _mm_unpacklo_epi32(__exta, __sign);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_epi64(__m128i __a)
{
  const __m128i __sign = _mm_cmpgt_epi32(_mm_setzero_si128(), __a);
  return _mm_unpacklo_epi32(__a, __sign);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu8_epi16(__m128i __a)
{
  return (__m128i)wasm_i16x8_widen_low_u8x16((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu8_epi32(__m128i __a)
{
  return (__m128i)wasm_i32x4_widen_low_u16x8(wasm_i16x8_widen_low_u8x16((v128_t)__a));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu8_epi64(__m128i __a)
{
  const __m128i __zero = _mm_setzero_si128();
  return _mm_unpacklo_epi32(_mm_unpacklo_epi16(_mm_unpacklo_epi8(__a, __zero), __zero), __zero);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu16_epi32(__m128i __a)
{
  return (__m128i)wasm_i32x4_widen_low_u16x8((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu16_epi64(__m128i __a)
{
  const __m128i __zero = _mm_setzero_si128();
  return _mm_unpacklo_epi32(_mm_unpacklo_epi16(__a, __zero), __zero);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtepu32_epi64(__m128i __a)
{
  const __m128i __zero = _mm_setzero_si128();
  return _mm_unpacklo_epi32(__a, __zero);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packus_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u16x8_narrow_i32x4(__a, __b);
}

static __inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
__uabs(int __i)
{
  return (unsigned short)((__i >= 0) ? __i : -__i);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mpsadbw_epu8(__m128i __a, __m128i __b, int __imm8)
{
  int __aOffset = __imm8 & 4;
  int __bOffset = (__imm8 & 3) << 2;
  unsigned short __ret[8];
  for(int __i = 0; __i < 8; ++__i)
  {
    __ret[__i] = __uabs(((__u8x16)__a)[__i + __aOffset    ] - ((__u8x16)__b)[__bOffset    ])
               + __uabs(((__u8x16)__a)[__i + __aOffset + 1] - ((__u8x16)__b)[__bOffset + 1])
               + __uabs(((__u8x16)__a)[__i + __aOffset + 2] - ((__u8x16)__b)[__bOffset + 2])
               + __uabs(((__u8x16)__a)[__i + __aOffset + 3] - ((__u8x16)__b)[__bOffset + 3]);
  }
  return (__m128i)wasm_v128_load(__ret);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_minpos_epu16(__m128i __a)
{
  unsigned short __min[2] = { 0xFFFF, 0 };
  for(int __i = 0; __i < 8; ++__i)
  {
    unsigned short __v = ((__u16x8)__a)[__i];
    if (__v < __min[0])
    {
      __min[0] = __v;
      __min[1] = __i;
    }
  }
  return (__m128i)wasm_i32x4_make(*(int*)__min, 0, 0, 0);
}

// Clang and GCC compatibility: Both Clang and GCC include SSE4.2 headers from SSE4.1 headers
#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

#endif /* __emscripten_smmintrin_h__ */
