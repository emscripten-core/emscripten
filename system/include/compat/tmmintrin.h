/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_tmmintrin_h__
#define __emscripten_tmmintrin_h__

#ifndef __SSSE3__
#error "SSSE3 instruction set not enabled"
#endif

#include <pmmintrin.h>

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi8(__m128i __a)
{
  return (__m128i)wasm_i8x16_abs((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi16(__m128i __a)
{
  return (__m128i)wasm_i16x8_abs((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi32(__m128i __a)
{
  return (__m128i)wasm_i32x4_abs((v128_t)__a);
}

#define _mm_alignr_epi8(__a, __b, __count) \
    ((__count <= 16) \
    ? (_mm_or_si128(_mm_bslli_si128((__a), 16 - (((unsigned int)(__count)) & 0xFF)), _mm_bsrli_si128((__b), (((unsigned int)(__count)) & 0xFF)))) \
    : (_mm_bsrli_si128((__a), (((unsigned int)(__count)) & 0xFF) - 16)))

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi16(__m128i __a, __m128i __b)
{
  return _mm_add_epi16((__m128i)wasm_i16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                       (__m128i)wasm_i16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi32(__m128i __a, __m128i __b)
{
  return _mm_add_epi32((__m128i)_mm_shuffle_ps((__m128)__a, (__m128)__b, _MM_SHUFFLE(2, 0, 2, 0)),
                       (__m128i)_mm_shuffle_ps((__m128)__a, (__m128)__b, _MM_SHUFFLE(3, 1, 3, 1)));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadds_epi16(__m128i __a, __m128i __b)
{
  return _mm_adds_epi16((__m128i)wasm_i16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                        (__m128i)wasm_i16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi16(__m128i __a, __m128i __b)
{
  return _mm_sub_epi16((__m128i)wasm_i16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                       (__m128i)wasm_i16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi32(__m128i __a, __m128i __b)
{
  return _mm_sub_epi32((__m128i)_mm_shuffle_ps((__m128)__a, (__m128)__b, _MM_SHUFFLE(2, 0, 2, 0)),
                       (__m128i)_mm_shuffle_ps((__m128)__a, (__m128)__b, _MM_SHUFFLE(3, 1, 3, 1)));
}


static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsubs_epi16(__m128i __a, __m128i __b)
{
  return _mm_subs_epi16((__m128i)wasm_i16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                        (__m128i)wasm_i16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_maddubs_epi16(__m128i __a, __m128i __b)
{
  return _mm_adds_epi16(
    _mm_mullo_epi16(
      _mm_and_si128(__a, _mm_set1_epi16(0x00FF)),
      _mm_srai_epi16(_mm_slli_epi16(__b, 8), 8)),
    _mm_mullo_epi16(_mm_srli_epi16(__a, 8), _mm_srai_epi16(__b, 8)));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhrs_epi16(__m128i __a, __m128i __b)
{
  v128_t __lo = wasm_i32x4_mul(wasm_i32x4_widen_low_i16x8((v128_t)__a), wasm_i32x4_widen_low_i16x8((v128_t)__b));
  v128_t __hi = wasm_i32x4_mul(wasm_i32x4_widen_high_i16x8((v128_t)__a), wasm_i32x4_widen_high_i16x8((v128_t)__b));
  const v128_t __inc = wasm_i32x4_splat(0x4000);
  __lo = wasm_i32x4_add(__lo, __inc);
  __hi = wasm_i32x4_add(__hi, __inc);
  __lo = wasm_i32x4_add(__lo, __lo);
  __hi = wasm_i32x4_add(__hi, __hi);
  return (__m128i)wasm_i16x8_shuffle(__lo, __hi, 1, 3, 5, 7, 9, 11, 13, 15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_shuffle_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_v8x16_swizzle((v128_t)__a, (v128_t)_mm_and_si128(__b, _mm_set1_epi8(0x8F)));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi8(__m128i __a, __m128i __b)
{
  const __m128i __zero = _mm_setzero_si128();
  __a = _mm_andnot_si128(_mm_cmpeq_epi8(__b, __zero), __a);
  const __m128i __mask = _mm_cmpgt_epi8(__zero, __b);
  return _mm_xor_si128(_mm_add_epi8(__a, __mask), __mask);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi16(__m128i __a, __m128i __b)
{
  const __m128i __zero = _mm_setzero_si128();
  __a = _mm_andnot_si128(_mm_cmpeq_epi16(__b, __zero), __a);
  const __m128i __mask = _mm_cmpgt_epi16(__zero, __b);
  return _mm_xor_si128(_mm_add_epi16(__a, __mask), __mask);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi32(__m128i __a, __m128i __b)
{
  const __m128i __zero = _mm_setzero_si128();
  __a = _mm_andnot_si128(_mm_cmpeq_epi32(__b, __zero), __a);
  const __m128i __mask = _mm_cmpgt_epi32(__zero, __b);
  return _mm_xor_si128(_mm_add_epi32(__a, __mask), __mask);
}

// Unavailable functions:
// _mm_abs_pi8
// _mm_abs_pi16
// _mm_abs_pi32
// _mm_alignr_pi8
// _mm_hadd_pi16
// _mm_hadd_pi32
// _mm_hadds_pi16
// _mm_hsub_pi16
// _mm_hsub_pi32
// _mm_hsubs_pi16
// _mm_maddubs_pi16
// _mm_mulhrs_pi16
// _mm_shuffle_pi8
// _mm_sign_pi8
// _mm_sign_pi16
// _mm_sign_pi32

#endif /* __emscripten_tmmintrin_h__ */
