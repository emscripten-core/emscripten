/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_pmmintrin_h__
#define __emscripten_pmmintrin_h__

#ifndef __SSE3__
#error "SSE3 instruction set not enabled"
#endif

#include <emmintrin.h>

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_lddqu_si128(__m128i const *__p)
{
  return _mm_loadu_si128(__p);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_addsub_ps(__m128 __a, __m128 __b)
{
  return _mm_add_ps(__a, _mm_mul_ps(__b, _mm_set_ps(1.f, -1.f, 1.f, -1.f)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_hadd_ps(__m128 __a, __m128 __b)
{
  return _mm_add_ps(_mm_shuffle_ps(__a, __b, _MM_SHUFFLE(2, 0, 2, 0)), _mm_shuffle_ps(__a, __b, _MM_SHUFFLE(3, 1, 3, 1)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_hsub_ps(__m128 __a, __m128 __b)
{
  return _mm_sub_ps(_mm_shuffle_ps(__a, __b, _MM_SHUFFLE(2, 0, 2, 0)), _mm_shuffle_ps(__a, __b, _MM_SHUFFLE(3, 1, 3, 1)));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movehdup_ps(__m128 __a)
{
  return (__m128)wasm_i32x4_shuffle(__a, __a, 1, 1, 3, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_moveldup_ps(__m128 __a)
{
  return (__m128)wasm_i32x4_shuffle(__a, __a, 0, 0, 2, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_addsub_pd(__m128d __a, __m128d __b)
{
  return _mm_add_pd(__a, _mm_mul_pd(__b, _mm_set_pd(1.0, -1.0)));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_hadd_pd(__m128d __a, __m128d __b)
{
  return _mm_add_pd(_mm_shuffle_pd(__a, __b, _MM_SHUFFLE2(0, 0)), _mm_shuffle_pd(__a, __b, _MM_SHUFFLE2(1, 1)));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_hsub_pd(__m128d __a, __m128d __b)
{
  return _mm_sub_pd(_mm_shuffle_pd(__a, __b, _MM_SHUFFLE2(0, 0)), _mm_shuffle_pd(__a, __b, _MM_SHUFFLE2(1, 1)));
}

#define        _mm_loaddup_pd(dp)        _mm_load1_pd(dp)

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_movedup_pd(__m128d __a)
{
  return (__m128d)wasm_i64x2_shuffle(__a, __a, 0, 0);
}

#define _MM_DENORMALS_ZERO_ON   (0x0040)
#define _MM_DENORMALS_ZERO_OFF  (0x0000)
#define _MM_DENORMALS_ZERO_MASK (0x0040)
#define _MM_GET_DENORMALS_ZERO_MODE() (_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)

// Unavailable functions:
// #define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
// void _mm_monitor(void const *__p, unsigned __extensions, unsigned __hints);
// void _mm_mwait(unsigned __extensions, unsigned __hints);

#endif /* __emscripten_pmmintrin_h__ */
