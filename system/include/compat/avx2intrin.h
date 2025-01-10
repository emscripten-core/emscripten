/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __emscripten_immintrin_h__
#error "Never use <avx2intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __emscripten_avx2intrin_h__
#define __emscripten_avx2intrin_h__

#ifndef __AVX2__
#error "AVX2 instruction set not enabled"
#endif

#define _mm256_mpsadbw_epu8(__A, __B, __imm)                                   \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    __m256i __b = (__B);                                                       \
    _mm256_set_m128i(_mm_mpsadbw_epu8(__a.v1, __b.v1, (__imm) >> 3),           \
                     _mm_mpsadbw_epu8(__a.v0, __b.v0, (__imm)));               \
  })

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_abs_epi8(__m256i __a) {
  __m256i ret;
  ret.v0 = _mm_abs_epi8(__a.v0);
  ret.v1 = _mm_abs_epi8(__a.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_abs_epi16(__m256i __a) {
  __m256i ret;
  ret.v0 = _mm_abs_epi16(__a.v0);
  ret.v1 = _mm_abs_epi16(__a.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_abs_epi32(__m256i __a) {
  __m256i ret;
  ret.v0 = _mm_abs_epi32(__a.v0);
  ret.v1 = _mm_abs_epi32(__a.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_packs_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_packs_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_packs_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_packs_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_packs_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_packs_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_packus_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_packus_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_packus_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_packus_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_packus_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_packus_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_add_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_add_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_add_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_add_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_add_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_add_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_add_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_add_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_add_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_add_epi64(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_add_epi64(__a.v0, __b.v0);
  ret.v1 = _mm_add_epi64(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_adds_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_adds_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_adds_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_adds_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_adds_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_adds_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_adds_epu8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_adds_epu8(__a.v0, __b.v0);
  ret.v1 = _mm_adds_epu8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_adds_epu16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_adds_epu16(__a.v0, __b.v0);
  ret.v1 = _mm_adds_epu16(__a.v1, __b.v1);
  return ret;
}

#define _mm256_alignr_epi8(__A, __B, __imm)                                    \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    __m256i __b = (__B);                                                       \
    _mm256_set_m128i(_mm_alignr_epi8(__a.v1, __b.v1, (__imm)),                 \
                     _mm_alignr_epi8(__a.v0, __b.v0, (__imm)));                \
  })

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_and_si256(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_and_si128(__a.v0, __b.v0);
  ret.v1 = _mm_and_si128(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_andnot_si256(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_andnot_si128(__a.v0, __b.v0);
  ret.v1 = _mm_andnot_si128(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_avg_epu8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_avg_epu8(__a.v0, __b.v0);
  ret.v1 = _mm_avg_epu8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_avg_epu16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_avg_epu16(__a.v0, __b.v0);
  ret.v1 = _mm_avg_epu16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_blendv_epi8(__m256i __a, __m256i __b, __m256i __mask) {
  __m256i ret;
  ret.v0 = _mm_blendv_epi8(__a.v0, __b.v0, __mask.v0);
  ret.v1 = _mm_blendv_epi8(__a.v1, __b.v1, __mask.v1);
  return ret;
}

#define _mm256_blend_epi16(__A, __B, __imm)                                    \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    __m256i __b = (__B);                                                       \
    _mm256_set_m128i(_mm_blend_epi16(__a.v1, __b.v1, (__imm)),                 \
                     _mm_blend_epi16(__a.v0, __b.v0, (__imm)));                \
  })

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpeq_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpeq_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_cmpeq_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpeq_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpeq_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_cmpeq_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpeq_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpeq_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_cmpeq_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpeq_epi64(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpeq_epi64(__a.v0, __b.v0);
  ret.v1 = _mm_cmpeq_epi64(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpgt_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpgt_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_cmpgt_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpgt_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpgt_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_cmpgt_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpgt_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpgt_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_cmpgt_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cmpgt_epi64(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_cmpgt_epi64(__a.v0, __b.v0);
  ret.v1 = _mm_cmpgt_epi64(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_hadd_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_hadd_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_hadd_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_hadd_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_hadd_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_hadd_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_hadds_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_hadds_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_hadds_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_hsub_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_hsub_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_hsub_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_hsub_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_hsub_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_hsub_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_hsubs_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_hsubs_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_hsubs_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_maddubs_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_maddubs_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_maddubs_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_madd_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_madd_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_madd_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_max_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_max_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_max_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_max_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_max_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_max_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_max_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_max_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_max_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_max_epu8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_max_epu8(__a.v0, __b.v0);
  ret.v1 = _mm_max_epu8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_max_epu16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_max_epu16(__a.v0, __b.v0);
  ret.v1 = _mm_max_epu16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_max_epu32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_max_epu32(__a.v0, __b.v0);
  ret.v1 = _mm_max_epu32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_min_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_min_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_min_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_min_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_min_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_min_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_min_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_min_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_min_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_min_epu8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_min_epu8(__a.v0, __b.v0);
  ret.v1 = _mm_min_epu8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_min_epu16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_min_epu16(__a.v0, __b.v0);
  ret.v1 = _mm_min_epu16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_min_epu32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_min_epu32(__a.v0, __b.v0);
  ret.v1 = _mm_min_epu32(__a.v1, __b.v1);
  return ret;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_movemask_epi8(__m256i __a) {
  return (_mm_movemask_epi8(__a.v1) << 16) | _mm_movemask_epi8(__a.v0);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi8_epi16(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepi8_epi16(__a);
  ret.v1 = _mm_cvtepi8_epi16(_mm_shuffle_epi32(__a, 0x4E));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi8_epi32(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepi8_epi32(__a);
  ret.v1 = _mm_cvtepi8_epi32(_mm_shuffle_epi32(__a, 0xE1));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi8_epi64(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepi8_epi64(__a);
  ret.v1 = _mm_cvtepi8_epi64(_mm_srli_epi32(__a, 16));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi16_epi32(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepi16_epi32(__a);
  ret.v1 = _mm_cvtepi16_epi32(_mm_shuffle_epi32(__a, 0x4E));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi16_epi64(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepi16_epi64(__a);
  ret.v1 = _mm_cvtepi16_epi64(_mm_shuffle_epi32(__a, 0xE1));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi32_epi64(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepi32_epi64(__a);
  ret.v1 = _mm_cvtepi32_epi64(_mm_shuffle_epi32(__a, 0x4E));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepu8_epi16(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepu8_epi16(__a);
  ret.v1 = _mm_cvtepu8_epi16(_mm_shuffle_epi32(__a, 0x4E));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepu8_epi32(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepu8_epi32(__a);
  ret.v1 = _mm_cvtepu8_epi32(_mm_shuffle_epi32(__a, 0xE1));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepu8_epi64(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepu8_epi64(__a);
  ret.v1 = _mm_cvtepu8_epi64(_mm_srli_epi32(__a, 16));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepu16_epi32(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepu16_epi32(__a);
  ret.v1 = _mm_cvtepu16_epi32(_mm_shuffle_epi32(__a, 0x4E));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepu16_epi64(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepu16_epi64(__a);
  ret.v1 = _mm_cvtepu16_epi64(_mm_shuffle_epi32(__a, 0xE1));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepu32_epi64(__m128i __a) {
  __m256i ret;
  ret.v0 = _mm_cvtepu32_epi64(__a);
  ret.v1 = _mm_cvtepu32_epi64(_mm_shuffle_epi32(__a, 0x4E));
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mul_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mul_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_mul_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mulhrs_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mulhrs_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_mulhrs_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mulhi_epu16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mulhi_epu16(__a.v0, __b.v0);
  ret.v1 = _mm_mulhi_epu16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mulhi_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mulhi_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_mulhi_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mullo_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mullo_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_mullo_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mullo_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mullo_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_mullo_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mul_epu32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_mul_epu32(__a.v0, __b.v0);
  ret.v1 = _mm_mul_epu32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_or_si256(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_or_si128(__a.v0, __b.v0);
  ret.v1 = _mm_or_si128(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sad_epu8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sad_epu8(__a.v0, __b.v0);
  ret.v1 = _mm_sad_epu8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_shuffle_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_shuffle_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_shuffle_epi8(__a.v1, __b.v1);
  return ret;
}

#define _mm256_shuffle_epi32(__A, __imm)                                       \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    _mm256_set_m128i(_mm_shuffle_epi32(__a.v1, (__imm)),                       \
                     _mm_shuffle_epi32(__a.v0, (__imm)));                      \
  })

#define _mm256_shufflehi_epi16(__A, __imm)                                     \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    _mm256_set_m128i(_mm_shufflehi_epi16(__a.v1, (__imm)),                     \
                     _mm_shufflehi_epi16(__a.v0, (__imm)));                    \
  })

#define _mm256_shufflelo_epi16(__A, __imm)                                     \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    _mm256_set_m128i(_mm_shufflelo_epi16(__a.v1, (__imm)),                     \
                     _mm_shufflelo_epi16(__a.v0, (__imm)));                    \
  })

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sign_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sign_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_sign_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sign_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sign_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_sign_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sign_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sign_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_sign_epi32(__a.v1, __b.v1);
  return ret;
}

#define _mm256_slli_si256(__A, __imm)                                          \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    _mm256_set_m128i(_mm_slli_si128(__a.v1, (__imm)),                          \
                     _mm_slli_si128(__a.v0, (__imm)));                         \
  })

#define _mm256_bslli_epi128(__A, __imm) _mm256_slli_si256(__A, __imm)

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_slli_epi16(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_slli_epi16(__a.v0, __count);
  ret.v1 = _mm_slli_epi16(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sll_epi16(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_sll_epi16(__a.v0, __count);
  ret.v1 = _mm_sll_epi16(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_slli_epi32(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_slli_epi32(__a.v0, __count);
  ret.v1 = _mm_slli_epi32(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sll_epi32(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_sll_epi32(__a.v0, __count);
  ret.v1 = _mm_sll_epi32(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_slli_epi64(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_slli_epi64(__a.v0, __count);
  ret.v1 = _mm_slli_epi64(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sll_epi64(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_sll_epi64(__a.v0, __count);
  ret.v1 = _mm_sll_epi64(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srai_epi16(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_srai_epi16(__a.v0, __count);
  ret.v1 = _mm_srai_epi16(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sra_epi16(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_sra_epi16(__a.v0, __count);
  ret.v1 = _mm_sra_epi16(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srai_epi32(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_srai_epi32(__a.v0, __count);
  ret.v1 = _mm_srai_epi32(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sra_epi32(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_sra_epi32(__a.v0, __count);
  ret.v1 = _mm_sra_epi32(__a.v1, __count);
  return ret;
}

#define _mm256_srli_si256(__A, __imm)                                          \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    _mm256_set_m128i(_mm_srli_si128(__a.v1, (__imm)),                          \
                     _mm_srli_si128(__a.v0, (__imm)));                         \
  })

#define _mm256_bsrli_epi128(a, imm) _mm256_srli_si256(a, imm)

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srli_epi16(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_srli_epi16(__a.v0, __count);
  ret.v1 = _mm_srli_epi16(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srl_epi16(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_srl_epi16(__a.v0, __count);
  ret.v1 = _mm_srl_epi16(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srli_epi32(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_srli_epi32(__a.v0, __count);
  ret.v1 = _mm_srli_epi32(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srl_epi32(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_srl_epi32(__a.v0, __count);
  ret.v1 = _mm_srl_epi32(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srli_epi64(__m256i __a, int __count) {
  __m256i ret;
  ret.v0 = _mm_srli_epi64(__a.v0, __count);
  ret.v1 = _mm_srli_epi64(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_srl_epi64(__m256i __a, __m128i __count) {
  __m256i ret;
  ret.v0 = _mm_srl_epi64(__a.v0, __count);
  ret.v1 = _mm_srl_epi64(__a.v1, __count);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sub_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sub_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_sub_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sub_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sub_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_sub_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sub_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sub_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_sub_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_sub_epi64(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_sub_epi64(__a.v0, __b.v0);
  ret.v1 = _mm_sub_epi64(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_subs_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_subs_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_subs_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_subs_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_subs_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_subs_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_subs_epu8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_subs_epu8(__a.v0, __b.v0);
  ret.v1 = _mm_subs_epu8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_subs_epu16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_subs_epu16(__a.v0, __b.v0);
  ret.v1 = _mm_subs_epu16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpackhi_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpackhi_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_unpackhi_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpackhi_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpackhi_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_unpackhi_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpackhi_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpackhi_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_unpackhi_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpackhi_epi64(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpackhi_epi64(__a.v0, __b.v0);
  ret.v1 = _mm_unpackhi_epi64(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpacklo_epi8(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpacklo_epi8(__a.v0, __b.v0);
  ret.v1 = _mm_unpacklo_epi8(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpacklo_epi16(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpacklo_epi16(__a.v0, __b.v0);
  ret.v1 = _mm_unpacklo_epi16(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpacklo_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpacklo_epi32(__a.v0, __b.v0);
  ret.v1 = _mm_unpacklo_epi32(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_unpacklo_epi64(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_unpacklo_epi64(__a.v0, __b.v0);
  ret.v1 = _mm_unpacklo_epi64(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_xor_si256(__m256i __a, __m256i __b) {
  __m256i ret;
  ret.v0 = _mm_xor_si128(__a.v0, __b.v0);
  ret.v1 = _mm_xor_si128(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_stream_load_si256(const void* __V) {
  __m256i ret;
  ret.v0 = _mm_stream_load_si128((const __m128i*)__V);
  ret.v1 = _mm_stream_load_si128((const __m128i*)(((const uint8_t*)__V) + 16));
  return ret;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_broadcastss_ps(__m128 __a) {
  return (__m128)wasm_i32x4_shuffle(__a, __a, 0, 0, 0, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_broadcastsd_pd(__m128d __a) {
  return (__m128d)wasm_i64x2_shuffle(__a, __a, 0, 0);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastss_ps(__m128 __a) {
  __m256 ret;
  ret.v1 = ret.v0 = _mm_broadcastss_ps(__a);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastsd_pd(__m128d __a) {
  __m256d ret;
  ret.v1 = ret.v0 = _mm_broadcastsd_pd(__a);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastsi128_si256(__m128i __a) {
  __m256i ret;
  ret.v1 = ret.v0 = __a;
  return ret;
}

#define _mm_broadcastsi128_si256(X) _mm256_broadcastsi128_si256(X)

#define _mm_blend_epi32(__a, __b, __imm8)                                      \
  __extension__({                                                              \
    (__m128i) __builtin_shufflevector((__i32x4)(__m128i)(__a),                 \
                                      (__i32x4)(__m128i)(__b),                 \
                                      (((__imm8) & 0x01) ? 4 : 0),             \
                                      (((__imm8) & 0x02) ? 5 : 1),             \
                                      (((__imm8) & 0x04) ? 6 : 2),             \
                                      (((__imm8) & 0x08) ? 7 : 3));            \
  })

#define _mm256_blend_epi32(__A, __B, __imm)                                    \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    __m256i __b = (__B);                                                       \
    _mm256_set_m128i(_mm_blend_epi32(__a.v1, __b.v1, (__imm) >> 4),            \
                     _mm_blend_epi32(__a.v0, __b.v0, (__imm)));                \
  })

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_broadcastb_epi8(__m128i __a) {
  return (__m128i)wasm_i8x16_shuffle(
    __a, __a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_broadcastw_epi16(__m128i __a) {
  return (__m128i)wasm_i16x8_shuffle(__a, __a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_broadcastd_epi32(__m128i __a) {
  return (__m128i)wasm_i32x4_shuffle(__a, __a, 0, 0, 0, 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_broadcastq_epi64(__m128i __a) {
  return (__m128i)wasm_i64x2_shuffle(__a, __a, 0, 0);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastb_epi8(__m128i __a) {
  __m256i ret;
  ret.v1 = ret.v0 = _mm_broadcastb_epi8(__a);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastw_epi16(__m128i __a) {
  __m256i ret;
  ret.v1 = ret.v0 = _mm_broadcastw_epi16(__a);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastd_epi32(__m128i __a) {
  __m256i ret;
  ret.v1 = ret.v0 = _mm_broadcastd_epi32(__a);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_broadcastq_epi64(__m128i __a) {
  __m256i ret;
  ret.v1 = ret.v0 = _mm_broadcastq_epi64(__a);
  return ret;
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_permutevar8x32_epi32(__m256i __a, __m256i __b) {
  __m256i ret;
  int index[8];
  int lane[8];
  for (int i = 0; i < 4; i++) {
    index[i] = ((__i32x4)__b.v0)[i] & 7;
    index[i + 4] = ((__i32x4)__b.v1)[i] & 7;
  }

  for (int j = 0; j < 8; j++) {
    lane[j] = index[j] < 4 ? ((__i32x4)(__a.v0))[index[j]]
                           : ((__i32x4)(__a.v1))[index[j] - 4];
  }

  ret.v0 = (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
  ret.v1 = (__m128i)wasm_i32x4_make(lane[4], lane[5], lane[6], lane[7]);
  return ret;
}

#define _mm256_permute4x64_pd(__A, __imm)                                      \
  __extension__({                                                              \
    __m256d __a = (__A);                                                       \
    _mm256_set_m128d(                                                          \
      (__m128d)wasm_i64x2_shuffle(                                             \
        __a.v0, __a.v1, (((__imm) >> 4) & 3), (((__imm) >> 6) & 3)),           \
      (__m128d)wasm_i64x2_shuffle(                                             \
        __a.v0, __a.v1, ((__imm) & 3), (((__imm) >> 2) & 3)));                 \
  })

static __inline__ __m256
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_permutevar8x32_ps(__m256 __a, __m256i __b) {
  __m256 ret;
  int index[8];
  float lane[8];
  for (int i = 0; i < 4; i++) {
    index[i] = ((__i32x4)__b.v0)[i] & 7;
    index[i + 4] = ((__i32x4)__b.v1)[i] & 7;
  }
  for (int j = 0; j < 8; j++) {
    lane[j] = index[j] < 4 ? ((__f32x4)(__a.v0))[index[j]]
                           : ((__f32x4)(__a.v1))[index[j] - 4];
  }
  ret.v0 = (__m128)wasm_f32x4_make(lane[0], lane[1], lane[2], lane[3]);
  ret.v1 = (__m128)wasm_f32x4_make(lane[4], lane[5], lane[6], lane[7]);
  return ret;
}

#define _mm256_permute4x64_epi64(__A, __imm)                                   \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    _mm256_set_m128i(                                                          \
      wasm_i64x2_shuffle(                                                      \
        __a.v0, __a.v1, (((__imm) >> 4) & 3), (((__imm) >> 6) & 3)),           \
      wasm_i64x2_shuffle(                                                      \
        __a.v0, __a.v1, ((__imm) & 3), (((__imm) >> 2) & 3)));                 \
  })

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_permute2x128_si256(__m256i __a, __m256i __b, const int imm8) {
  __m256i ret;
  ret.v0 = __avx_select4i(__a, __b, imm8);
  ret.v1 = __avx_select4i(__a, __b, imm8 >> 4);
  return ret;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm256_extracti128_si256(__m256i __a, const int imm8) {
  if (imm8 & 0x1) {
    return __a.v1;
  } else {
    return __a.v0;
  }
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_inserti128_si256(__m256i __a, __m128i __b, const int imm8) {
  __m256i ret = __a;
  if (imm8 & 0x1) {
    ret.v1 = __b;
  } else {
    ret.v0 = __b;
  }
  return ret;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_maskload_epi32(int32_t const* __p, __m128i __m) {
  int32_t lane[4];
  for (size_t i = 0; i < 4; i++) {
    uint32_t mask = ((__i32x4)__m)[i];
    lane[i] = ((mask >> 31) & 0x1) ? __p[i] : 0;
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_maskload_epi64(int64_t const* __p, __m128i __m) {
  int64_t lane[2];
  for (size_t i = 0; i < 2; i++) {
    uint64_t mask = ((__i64x2)__m)[i];
    lane[i] = ((mask >> 63) & 0x1) ? __p[i] : 0;
  }
  return (__m128i)wasm_i64x2_make(lane[0], lane[1]);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_epi32(int const* __p, __m256i __m) {
  __m256i ret;
  ret.v0 = _mm_maskload_epi32(__p, __m.v0);
  ret.v1 = _mm_maskload_epi32(((int32_t*)__p) + 4, __m.v1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_epi64(long long const* __p, __m256i __m) {
  __m256i ret;
  ret.v0 = _mm_maskload_epi64(__p, __m.v0);
  ret.v1 = _mm_maskload_epi64(((int64_t*)__p) + 2, __m.v1);
  return ret;
}

static __inline__ void
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_maskstore_epi32(int* __p, __m128i __m, __m128i __a) {
  if ((wasm_i32x4_extract_lane(__m, 0) & 0x80000000ull) != 0)
    __p[0] = wasm_i32x4_extract_lane((v128_t)__a, 0);
  if ((wasm_i32x4_extract_lane(__m, 1) & 0x80000000ull) != 0)
    __p[1] = wasm_i32x4_extract_lane((v128_t)__a, 1);
  if ((wasm_i32x4_extract_lane(__m, 2) & 0x80000000ull) != 0)
    __p[2] = wasm_i32x4_extract_lane((v128_t)__a, 2);
  if ((wasm_i32x4_extract_lane(__m, 3) & 0x80000000ull) != 0)
    __p[3] = wasm_i32x4_extract_lane((v128_t)__a, 3);
}

static __inline__ void
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_maskstore_epi64(long long* __p, __m128i __m, __m128i __a) {
  if ((wasm_i64x2_extract_lane(__m, 0) & 0x8000000000000000ull) != 0)
    __p[0] = wasm_i64x2_extract_lane((v128_t)__a, 0);
  if ((wasm_i64x2_extract_lane(__m, 1) & 0x8000000000000000ull) != 0)
    __p[1] = wasm_i64x2_extract_lane((v128_t)__a, 1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_epi32(int* __p, __m256i __m, __m256i __a) {
  _mm_maskstore_epi32(__p, __m.v0, __a.v0);
  _mm_maskstore_epi32(((int32_t*)__p) + 4, __m.v1, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_epi64(long long* __p, __m256i __m, __m256i __a) {
  _mm_maskstore_epi64(__p, __m.v0, __a.v0);
  _mm_maskstore_epi64(((int64_t*)__p) + 2, __m.v1, __a.v1);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_sllv_epi32(__m128i __a, __m128i __count) {
  int32_t lane[4];
  for (size_t i = 0; i < 4; i++) {
    uint32_t shift = ((__u32x4)__count)[i];
    lane[i] = shift < 32 ? ((__u32x4)__a)[i] << shift : 0;
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_sllv_epi32(__m256i __a, __m256i __count) {
  __m256i ret;
  ret.v0 = _mm_sllv_epi32(__a.v0, __count.v0);
  ret.v1 = _mm_sllv_epi32(__a.v1, __count.v1);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_sllv_epi64(__m128i __a, __m128i __count) {

  int64_t lane[2];
  for (size_t i = 0; i < 2; i++) {
    uint64_t shift = (uint64_t)((__u64x2)__count)[i];
    lane[i] = shift < 64 ? ((__u64x2)__a)[i] << shift : 0;
  }
  return (__m128i)wasm_i64x2_make(lane[0], lane[1]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_sllv_epi64(__m256i __a, __m256i __count) {
  __m256i ret;
  ret.v0 = _mm_sllv_epi64(__a.v0, __count.v0);
  ret.v1 = _mm_sllv_epi64(__a.v1, __count.v1);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_srav_epi32(__m128i __a, __m128i __count) {
  int32_t lane[4];
  for (size_t i = 0; i < 4; i++) {
    uint32_t shift = ((__u32x4)__count)[i];
    shift = shift < 31 ? shift : 31;
    lane[i] = ((__i32x4)__a)[i] >> shift;
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_srav_epi32(__m256i __a, __m256i __count) {
  __m256i ret;
  ret.v0 = _mm_srav_epi32(__a.v0, __count.v0);
  ret.v1 = _mm_srav_epi32(__a.v1, __count.v1);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_srlv_epi32(__m128i __a, __m128i __count) {
  int32_t lane[4];
  for (size_t i = 0; i < 4; i++) {
    uint32_t shift = ((__u32x4)__count)[i];
    lane[i] = shift < 32 ? ((__u32x4)__a)[i] >> shift : 0;
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_srlv_epi32(__m256i __a, __m256i __count) {
  __m256i ret;
  ret.v0 = _mm_srlv_epi32(__a.v0, __count.v0);
  ret.v1 = _mm_srlv_epi32(__a.v1, __count.v1);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_srlv_epi64(__m128i __a, __m128i __count) {
  int64_t lane[2];
  for (size_t i = 0; i < 2; i++) {
    uint64_t shift = ((__u64x2)__count)[i];
    lane[i] = shift < 64 ? ((__u64x2)__a)[i] >> shift : 0;
  }
  return (__m128i)wasm_i64x2_make(lane[0], lane[1]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_srlv_epi64(__m256i __a, __m256i __count) {
  __m256i ret;
  ret.v0 = _mm_srlv_epi64(__a.v0, __count.v0);
  ret.v1 = _mm_srlv_epi64(__a.v1, __count.v1);
  return ret;
}

static __inline__ __m128d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i32gather_pd(__m128d src,
                        const double* base_addr,
                        __m128i vindex,
                        __m128d mask,
                        const int scale) {
  double lane[2];
  for (size_t i = 0; i < 2; i++) {
    if ((((__i64x2)mask)[i] >> 63) & 0x1) {
      double* addr =
        (double*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                          (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__f64x2)src)[i];
    }
  }
  return (__m128d)wasm_f64x2_make(lane[0], lane[1]);
}

static __inline__ __m256d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i32gather_pd(__m256d src,
                           const double* base_addr,
                           __m128i vindex,
                           __m256d mask,
                           const int scale) {
  __m256d ret;
  ret.v0 = _mm_mask_i32gather_pd(src.v0, base_addr, vindex, mask.v0, scale);
  __m128i vindex1 = (__m128i)wasm_i32x4_shuffle(vindex, vindex, 2, 3, 0, 1);
  ret.v1 = _mm_mask_i32gather_pd(src.v1, base_addr, vindex1, mask.v1, scale);
  return ret;
}

static __inline__ __m128d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i64gather_pd(__m128d src,
                        const double* base_addr,
                        __m128i vindex,
                        __m128d mask,
                        const int scale) {
  double lane[2];
  for (size_t i = 0; i < 2; i++) {
    if ((((__i64x2)mask)[i] >> 63) & 0x1) {
      double* addr =
        (double*)((uint8_t*)base_addr +
                  ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__f64x2)src)[i];
    }
  }
  return (__m128d)wasm_f64x2_make(lane[0], lane[1]);
}

static __inline__ __m256d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i64gather_pd(__m256d src,
                           const double* base_addr,
                           __m256i vindex,
                           __m256d mask,
                           const int scale) {
  __m256d ret;
  ret.v0 = _mm_mask_i64gather_pd(src.v0, base_addr, vindex.v0, mask.v0, scale);
  ret.v1 = _mm_mask_i64gather_pd(src.v1, base_addr, vindex.v1, mask.v1, scale);
  return ret;
}

static __inline__ __m128
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i32gather_ps(__m128 src,
                        const float* base_addr,
                        __m128i vindex,
                        __m128 mask,
                        const int scale) {
  float lane[4];
  for (size_t i = 0; i < 4; i++) {
    if ((((__i32x4)mask)[i] >> 31) & 0x1) {
      float* addr =
        (float*)((uint8_t*)base_addr +
                 (int64_t)(((__i32x4)vindex)[i]) * (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__f32x4)src)[i];
    }
  }
  return (__m128)wasm_f32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m256
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i32gather_ps(__m256 src,
                           const float* base_addr,
                           __m256i vindex,
                           __m256 mask,
                           const int scale) {
  __m256 ret;
  ret.v0 = _mm_mask_i32gather_ps(src.v0, base_addr, vindex.v0, mask.v0, scale);
  ret.v1 = _mm_mask_i32gather_ps(src.v1, base_addr, vindex.v1, mask.v1, scale);
  return ret;
}

static __inline__ __m128
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i64gather_ps(__m128 src,
                        const float* base_addr,
                        __m128i vindex,
                        __m128 mask,
                        const int scale) {
  float lane[2];
  for (size_t i = 0; i < 2; i++) {
    if ((((__i32x4)mask)[i] >> 31) & 0x1) {
      float* addr =
        (float*)((uint8_t*)base_addr +
                 ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__f32x4)src)[i];
    }
  }
  return (__m128)wasm_f32x4_make(lane[0], lane[1], 0, 0);
}

static __inline__ __m128
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i64gather_ps(__m128 src,
                           const float* base_addr,
                           __m256i vindex,
                           __m128 mask,
                           const int scale) {
  float lane[4];
  __m128i current_vindex;
  for (size_t i = 0; i < 4; i++) {
    current_vindex = i < 2 ? vindex.v0 : vindex.v1;
    if ((((__i32x4)mask)[i] >> 31) & 0x1) {
      float* addr =
        (float*)((uint8_t*)base_addr + ((__i64x2)current_vindex)[i & 1] *
                                         (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__f32x4)src)[i];
    }
  }
  return (__m128)wasm_f32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i32gather_epi32(__m128i src,
                           const int* base_addr,
                           __m128i vindex,
                           __m128i mask,
                           const int scale) {
  int32_t lane[4];
  for (size_t i = 0; i < 4; i++) {
    if ((((__i32x4)mask)[i] >> 31) & 0x1) {
      int32_t* addr =
        (int32_t*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                           (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__i32x4)src)[i];
    }
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i32gather_epi32(__m256i src,
                              const int* base_addr,
                              __m256i vindex,
                              __m256i mask,
                              const int scale) {
  __m256i ret;
  ret.v0 =
    _mm_mask_i32gather_epi32(src.v0, base_addr, vindex.v0, mask.v0, scale);
  ret.v1 =
    _mm_mask_i32gather_epi32(src.v1, base_addr, vindex.v1, mask.v1, scale);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i64gather_epi32(__m128i src,
                           const int* base_addr,
                           __m128i vindex,
                           __m128i mask,
                           const int scale) {
  int32_t lane[2];
  for (size_t i = 0; i < 2; i++) {
    if ((((__i32x4)mask)[i] >> 31) & 0x1) {
      int32_t* addr =
        (int32_t*)((uint8_t*)base_addr +
                   ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__i32x4)src)[i];
    }
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], 0, 0);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i64gather_epi32(__m128i src,
                              const int* base_addr,
                              __m256i vindex,
                              __m128i mask,
                              const int scale) {
  int32_t lane[4];
  __m128i current_vindex;
  for (size_t i = 0; i < 4; i++) {
    current_vindex = i < 2 ? vindex.v0 : vindex.v1;
    if ((((__i32x4)mask)[i] >> 31) & 0x1) {
      int32_t* addr =
        (int32_t*)((uint8_t*)base_addr + ((__i64x2)current_vindex)[i & 1] *
                                           (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__i32x4)src)[i];
    }
  }
  return (__m128i)wasm_i32x4_make(lane[0], lane[1], lane[2], lane[3]);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i32gather_epi64(__m128i src,
                           const long long* base_addr,
                           __m128i vindex,
                           __m128i mask,
                           const int scale) {
  int64_t lane[2];
  for (size_t i = 0; i < 2; i++) {
    if ((((__i64x2)mask)[i] >> 63) & 0x1) {
      int64_t* addr =
        (int64_t*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                           (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__i64x2)src)[i];
    }
  }
  return (__m128i)wasm_i64x2_make(lane[0], lane[1]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i32gather_epi64(__m256i src,
                              const long long* base_addr,
                              __m128i vindex,
                              __m256i mask,
                              const int scale) {
  __m256i ret;
  ret.v0 = _mm_mask_i32gather_epi64(src.v0, base_addr, vindex, mask.v0, scale);
  __m128i vindex1 = (__m128i)wasm_i32x4_shuffle(vindex, vindex, 2, 3, 0, 1);
  ret.v1 = _mm_mask_i32gather_epi64(src.v1, base_addr, vindex1, mask.v1, scale);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_mask_i64gather_epi64(__m128i src,
                           const long long* base_addr,
                           __m128i vindex,
                           __m128i mask,
                           const int scale) {
  int64_t lane[2];
  for (size_t i = 0; i < 2; i++) {
    if ((((__i64x2)mask)[i] >> 63) & 0x1) {
      int64_t* addr =
        (int64_t*)((uint8_t*)base_addr +
                   ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
      lane[i] = *addr;
    } else {
      lane[i] = ((__i64x2)src)[i];
    }
  }
  return (__m128i)wasm_i64x2_make(lane[0], lane[1]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_mask_i64gather_epi64(__m256i src,
                              const long long* base_addr,
                              __m256i vindex,
                              __m256i mask,
                              const int scale) {
  __m256i ret;
  ret.v0 =
    _mm_mask_i64gather_epi64(src.v0, base_addr, vindex.v0, mask.v0, scale);
  ret.v1 =
    _mm_mask_i64gather_epi64(src.v1, base_addr, vindex.v1, mask.v1, scale);
  return ret;
}

static __inline__ __m128d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i32gather_pd(const double* base_addr, __m128i vindex, const int scale) {
  double* lane[2];
  for (size_t i = 0; i < 2; i++) {
    lane[i] = (double*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                                (uint64_t)((uint32_t)scale));
  }
  return (__m128d)wasm_f64x2_make(*lane[0], *lane[1]);
}

static __inline__ __m256d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i32gather_pd(const double* base_addr,
                      __m128i vindex,
                      const int scale) {
  __m256d ret;
  double* lane[4];
  for (size_t i = 0; i < 4; i++) {
    lane[i] = (double*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                                (uint64_t)((uint32_t)scale));
  }
  ret.v0 = (__m128d)wasm_f64x2_make(*lane[0], *lane[1]);
  ret.v1 = (__m128d)wasm_f64x2_make(*lane[2], *lane[3]);
  return ret;
}

static __inline__ __m128d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i64gather_pd(const double* base_addr, __m128i vindex, const int scale) {
  double* lane[2];
  for (size_t i = 0; i < 2; i++) {
    lane[i] = (double*)((uint8_t*)base_addr +
                        ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
  }
  return (__m128d)wasm_f64x2_make(*lane[0], *lane[1]);
}

static __inline__ __m256d
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i64gather_pd(const double* base_addr,
                      __m256i vindex,
                      const int scale) {
  __m256d ret;
  ret.v0 = _mm_i64gather_pd(base_addr, vindex.v0, scale);
  ret.v1 = _mm_i64gather_pd(base_addr, vindex.v1, scale);
  return ret;
}

static __inline__ __m128
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i32gather_ps(const float* base_addr, __m128i vindex, const int scale) {
  float* lane[4];
  for (size_t i = 0; i < 4; i++) {
    lane[i] = (float*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                               (uint64_t)((uint32_t)scale));
  }
  return (__m128)wasm_f32x4_make(*lane[0], *lane[1], *lane[2], *lane[3]);
}

static __inline__ __m256
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i32gather_ps(const float* base_addr, __m256i vindex, const int scale) {
  __m256 ret;
  ret.v0 = _mm_i32gather_ps(base_addr, vindex.v0, scale);
  ret.v1 = _mm_i32gather_ps(base_addr, vindex.v1, scale);
  return ret;
}

static __inline__ __m128
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i64gather_ps(const float* base_addr, __m128i vindex, const int scale) {
  float* lane[2];
  for (size_t i = 0; i < 2; i++) {
    lane[i] = (float*)((uint8_t*)base_addr +
                       ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
  }
  return (__m128)wasm_f32x4_make(*lane[0], *lane[1], 0, 0);
}

static __inline__ __m128
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i64gather_ps(const float* base_addr, __m256i vindex, const int scale) {
  float* lane[4];
  __m128i current_vindex;
  for (size_t i = 0; i < 4; i++) {
    current_vindex = i < 2 ? vindex.v0 : vindex.v1;
    lane[i] = (float*)((uint8_t*)base_addr + ((__i64x2)current_vindex)[i & 1] *
                                               (uint64_t)((uint32_t)scale));
  }
  return (__m128)wasm_f32x4_make(*lane[0], *lane[1], *lane[2], *lane[3]);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i32gather_epi32(const int* base_addr, __m128i vindex, const int scale) {
  int32_t* lane[4];
  for (size_t i = 0; i < 4; i++) {
    lane[i] = (int32_t*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                                 (uint64_t)((uint32_t)scale));
  }
  return (__m128i)wasm_i32x4_make(*lane[0], *lane[1], *lane[2], *lane[3]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i32gather_epi32(const int* base_addr,
                         __m256i vindex,
                         const int scale) {
  __m256i ret;
  ret.v0 = _mm_i32gather_epi32(base_addr, vindex.v0, scale);
  ret.v1 = _mm_i32gather_epi32(base_addr, vindex.v1, scale);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i64gather_epi32(const int* base_addr, __m128i vindex, const int scale) {
  int32_t* lane[2];
  for (size_t i = 0; i < 2; i++) {
    lane[i] = (int32_t*)((uint8_t*)base_addr +
                         ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
  }
  return (__m128i)wasm_i32x4_make(*lane[0], *lane[1], 0, 0);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i64gather_epi32(const int* base_addr,
                         __m256i vindex,
                         const int scale) {
  int32_t* lane[4];
  __m128i current_vindex;
  for (size_t i = 0; i < 4; i++) {
    current_vindex = i < 2 ? vindex.v0 : vindex.v1;
    lane[i] =
      (int32_t*)((uint8_t*)base_addr + ((__i64x2)current_vindex)[i & 1] *
                                         (uint64_t)((uint32_t)scale));
  }
  return (__m128i)wasm_i32x4_make(*lane[0], *lane[1], *lane[2], *lane[3]);
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i32gather_epi64(const long long* base_addr,
                      __m128i vindex,
                      const int scale) {
  int64_t* lane[2];
  for (size_t i = 0; i < 2; i++) {
    lane[i] = (int64_t*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                                 (uint64_t)((uint32_t)scale));
  }
  return (__m128i)wasm_i64x2_make(*lane[0], *lane[1]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i32gather_epi64(const long long* base_addr,
                         __m128i vindex,
                         const int scale) {

  __m256i ret;
  int64_t* lane[4];
  for (size_t i = 0; i < 4; i++) {
    lane[i] = (int64_t*)((uint8_t*)base_addr + (int64_t)(((__i32x4)vindex)[i]) *
                                                 (uint64_t)((uint32_t)scale));
  }
  ret.v0 = (__m128i)wasm_i64x2_make(*lane[0], *lane[1]);
  ret.v1 = (__m128i)wasm_i64x2_make(*lane[2], *lane[3]);
  return ret;
}

static __inline__ __m128i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_i64gather_epi64(const long long* base_addr,
                      __m128i vindex,
                      const int scale) {
  int64_t* lane[2];
  for (size_t i = 0; i < 2; i++) {
    lane[i] = (int64_t*)((uint8_t*)base_addr +
                         ((__i64x2)vindex)[i] * (uint64_t)((uint32_t)scale));
  }
  return (__m128i)wasm_i64x2_make(*lane[0], *lane[1]);
}

static __inline__ __m256i
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm256_i64gather_epi64(const long long* base_addr,
                         __m256i vindex,
                         const int scale) {
  __m256i ret;
  ret.v0 = _mm_i64gather_epi64(base_addr, vindex.v0, scale);
  ret.v1 = _mm_i64gather_epi64(base_addr, vindex.v1, scale);
  return ret;
}

#endif /* __emscripten_avx2intrin_h__ */
