/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_nmmintrin_h__
#define __emscripten_nmmintrin_h__

#ifndef __SSE4_2__
#error "SSE4.2 instruction set not enabled"
#endif

#include <smmintrin.h>

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi64(__m128i __a, __m128i __b)
{
  return (__m128i)((__i64x2)__a > (__i64x2)__b);
}

// Unsupported functions:
// _mm_cmpestra
// _mm_cmpestrc
// _mm_cmpestri
// _mm_cmpestrm
// _mm_cmpestro
// _mm_cmpestrs
// _mm_cmpestrz
// _mm_cmpistra
// _mm_cmpistrc
// _mm_cmpistri
// _mm_cmpistrm
// _mm_cmpistro
// _mm_cmpistrs
// _mm_cmpistrz
// _mm_crc32_u16
// _mm_crc32_u32
// _mm_crc32_u64
// _mm_crc32_u8

#endif /* __emscripten_nmmintrin_h__ */
