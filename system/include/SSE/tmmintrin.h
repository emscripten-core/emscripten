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
  __m128i __mask = (__m128i)wasm_i8x16_shr((v128_t)__a, 7);
  return _mm_xor_si128(_mm_add_epi8(__a, __mask), __mask);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi16(__m128i __a)
{
  __m128i __mask = _mm_srai_epi16(__a, 15);
  return _mm_xor_si128(_mm_add_epi16(__a, __mask), __mask);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi32(__m128i __a)
{
  __m128i __mask = _mm_srai_epi32(__a, 31);
  return _mm_xor_si128(_mm_add_epi32(__a, __mask), __mask);
}

#define _mm_alignr_epi8(__a, __b, __count) \
    ((__count <= 16) \
    ? (_mm_or_si128(_mm_bslli_si128((__a), 16 - (((unsigned int)(__count)) & 0xFF)), _mm_bsrli_si128((__b), (((unsigned int)(__count)) & 0xFF)))) \
    : (_mm_bsrli_si128((__a), (((unsigned int)(__count)) & 0xFF) - 16)))

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi16(__m128i __a, __m128i __b)
{
  return _mm_add_epi16((__m128i)wasm_v16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                       (__m128i)wasm_v16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
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
  return _mm_adds_epi16((__m128i)wasm_v16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                        (__m128i)wasm_v16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi16(__m128i __a, __m128i __b)
{
  return _mm_sub_epi16((__m128i)wasm_v16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                       (__m128i)wasm_v16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
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
  return _mm_subs_epi16((__m128i)wasm_v16x8_shuffle(__a, __b, 0, 2, 4, 6, 8, 10, 12, 14),
                        (__m128i)wasm_v16x8_shuffle(__a, __b, 1, 3, 5, 7, 9, 11, 13, 15));
}

static __inline__ short __attribute__((__always_inline__, __nodebug__))
__Saturate_To_Int16(int __x)
{
    return __x <= -32768 ? -32768 : (__x >= 32767 ? 32767 : __x);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_maddubs_epi16(__m128i __a, __m128i __b)
{
  union {
    char __x[16];
    short __s[8];
    __m128i __m;
  } __src, __src2, __dst;
  __src.__m = __a;
  __src2.__m = __b;
  for(int __i = 0; __i < 16; __i += 2)
      __dst.__s[__i>>1] = __Saturate_To_Int16((unsigned char)__src.__x[__i+1] * __src2.__x[__i+1] + (unsigned char)__src.__x[__i] * __src2.__x[__i]);
  return __dst.__m;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhrs_epi16(__m128i __a, __m128i __b)
{
  // TODO: the following sequence oughta work, but something is not quite right,
  // runtime test fails with

// Expected: _mm_mulhrs_epi16([0x9ABCDEF1,0x80000000,0x80808080,0x7F7F7F7F], [0xF9301AB9,0x80000000,0x40200000,0xC0200000]) = [0x0564F919,0x80000000,0xC0200000,0xC0600000]
//   Actual: _mm_mulhrs_epi16([0x9ABCDEF1,0x80000000,0x80808080,0x7F7F7F7F], [0xF9301AB9,0x80000000,0x40200000,0xC0200000]) = [0x0564F919,0x7FFF0000,0xC0200000,0xC0600000]

// Expected: _mm_mulhrs_epi16([0x9ABCDEF1,0x80000000,0x80808080,0x7F7F7F7F], [0xF9301AB9,0x80000000,0x40200000,0xC0200000]) = [0x0564F919,0x80000000,0xC0200000,0xC0600000]
//   Actual: _mm_mulhrs_epi16([0x9ABCDEF1,0x80000000,0x80808080,0x7F7F7F7F], [0xF9301AB9,0x80000000,0x40200000,0xC0200000]) = [0x0564F919,0x7FFF0000,0xC0200000,0xC0600000]

#if 0
  v128_t __lo = wasm_i32x4_mul(wasm_i32x4_widen_low_i16x8((v128_t)__a),
                               wasm_i32x4_widen_low_i16x8((v128_t)__b));

  v128_t __hi = wasm_i32x4_mul(wasm_i32x4_widen_high_i16x8((v128_t)__a),
                               wasm_i32x4_widen_high_i16x8((v128_t)__b));

  v128_t __one = wasm_i32x4_const(1, 1, 1, 1);

  __lo = wasm_i32x4_shr(wasm_i32x4_add(wasm_i32x4_shr(__lo, 14), __one), 1);
  __hi = wasm_i32x4_shr(wasm_i32x4_add(wasm_i32x4_shr(__hi, 14), __one), 1);

  return (__m128i)wasm_i16x8_narrow_i32x4((v128_t)__lo, (v128_t)__hi);
#else
  union {
    short __x[8];
    __m128i __m;
  } __src, __src2, __dst;
  __src.__m = __a;
  __src2.__m = __b;
  for(int __i = 0; __i < 8; ++__i)
      __dst.__x[__i] = (((__src.__x[__i] * __src2.__x[__i]) >> 14) + 1) >> 1;
  return __dst.__m;
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_shuffle_epi8(__m128i __a, __m128i __b)
{
  // TODO: use wasm_v8x16_swizzle() when it becomes available.
  union {
    unsigned char __x[16];
    __m128i __m;
  } __src, __src2, __dst;
  __src.__m = __a;
  __src2.__m = __b;
  for(int __i = 0; __i < 16; ++__i)
      __dst.__x[__i] = (__src2.__x[__i] & 0x80) ? 0 : __src.__x[__src2.__x[__i]&15];
  return __dst.__m;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi8(__m128i __a, __m128i __b)
{
  __m128i __mask = (__m128i)wasm_i8x16_shr((v128_t)__b, 7);
  __m128i __zeromask = _mm_cmpeq_epi8(__b, _mm_setzero_si128());
  return _mm_andnot_si128(__zeromask, _mm_xor_si128(_mm_add_epi8(__a, __mask), __mask));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi16(__m128i __a, __m128i __b)
{
  __m128i __mask = _mm_srai_epi16(__b, 15);
  __m128i __zeromask = _mm_cmpeq_epi16(__b, _mm_setzero_si128());
  return _mm_andnot_si128(__zeromask, _mm_xor_si128(_mm_add_epi16(__a, __mask), __mask));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi32(__m128i __a, __m128i __b)
{
  __m128i __mask = _mm_srai_epi32(__b, 31);
  __m128i __zeromask = _mm_cmpeq_epi32(__b, _mm_setzero_si128());
  return _mm_andnot_si128(__zeromask, _mm_xor_si128(_mm_add_epi32(__a, __mask), __mask));
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
