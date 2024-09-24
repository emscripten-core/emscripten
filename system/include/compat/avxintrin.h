/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_avxintrin_h__
#define __emscripten_avxintrin_h__

#ifndef __AVX__
#error "AVX instruction set not enabled"
#endif

#include <emmintrin.h>
#include <nmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>
#include <xmmintrin.h>

typedef struct {
  __m128d v0;
  __m128d v1;
} __m256d;

typedef struct {
  __m128 v0;
  __m128 v1;
} __m256;

typedef struct {
  __m128i v0;
  __m128i v1;
} __m256i;

typedef int64_t __m128i_u __attribute__((__vector_size__(16), __aligned__(1)));

typedef struct {
  __m128i_u v0;
  __m128i_u v1;
} __m256i_u;

union m256_data {
  __m256i int_view;
  __m256d double_view;
  __m256 float_view;
  __m128i_u int_u_view;
};

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_add_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_add_pd(__a.v0, __b.v0);
  ret.v1 = _mm_add_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_add_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_add_ps(__a.v0, __b.v0);
  ret.v1 = _mm_add_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_sub_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_sub_pd(__a.v0, __b.v0);
  ret.v1 = _mm_sub_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_sub_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_sub_ps(__a.v0, __b.v0);
  ret.v1 = _mm_sub_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_addsub_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_addsub_pd(__a.v0, __b.v0);
  ret.v1 = _mm_addsub_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_addsub_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_addsub_ps(__a.v0, __b.v0);
  ret.v1 = _mm_addsub_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_div_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_div_pd(__a.v0, __b.v0);
  ret.v1 = _mm_div_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_div_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_div_ps(__a.v0, __b.v0);
  ret.v1 = _mm_div_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_max_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_max_pd(__a.v0, __b.v0);
  ret.v1 = _mm_max_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_max_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_max_ps(__a.v0, __b.v0);
  ret.v1 = _mm_max_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_min_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_min_pd(__a.v0, __b.v0);
  ret.v1 = _mm_min_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_min_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_min_ps(__a.v0, __b.v0);
  ret.v1 = _mm_min_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_mul_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_mul_pd(__a.v0, __b.v0);
  ret.v1 = _mm_mul_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_mul_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_mul_ps(__a.v0, __b.v0);
  ret.v1 = _mm_mul_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_sqrt_pd(__m256d __a) {
  __m256d ret;
  ret.v0 = _mm_sqrt_pd(__a.v0);
  ret.v1 = _mm_sqrt_pd(__a.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_sqrt_ps(__m256 __a) {
  __m256 ret;
  ret.v0 = _mm_sqrt_ps(__a.v0);
  ret.v1 = _mm_sqrt_ps(__a.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_rsqrt_ps(__m256 __a) {
  __m256 ret;
  ret.v0 = _mm_rsqrt_ps(__a.v0);
  ret.v1 = _mm_rsqrt_ps(__a.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_rcp_ps(__m256 __a) {
  __m256 ret;
  ret.v0 = _mm_rcp_ps(__a.v0);
  ret.v1 = _mm_rcp_ps(__a.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_round_pd(__m256d __a, int __rounding) {
  __m256d ret;
  ret.v0 = _mm_round_pd(__a.v0, __rounding);
  ret.v1 = _mm_round_pd(__a.v1, __rounding);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_round_ps(__m256 __a, int __rounding) {
  __m256 ret;
  ret.v0 = _mm_round_ps(__a.v0, __rounding);
  ret.v1 = _mm_round_ps(__a.v1, __rounding);
  return ret;
}

#define _mm256_ceil_pd(V) _mm256_round_pd((V), _MM_FROUND_CEIL)
#define _mm256_floor_pd(V) _mm256_round_pd((V), _MM_FROUND_FLOOR)
#define _mm256_ceil_ps(V) _mm256_round_ps((V), _MM_FROUND_CEIL)
#define _mm256_floor_ps(V) _mm256_round_ps((V), _MM_FROUND_FLOOR)

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_and_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_and_pd(__a.v0, __b.v0);
  ret.v1 = _mm_and_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_and_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_and_ps(__a.v0, __b.v0);
  ret.v1 = _mm_and_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_andnot_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_andnot_pd(__a.v0, __b.v0);
  ret.v1 = _mm_andnot_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_andnot_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_andnot_ps(__a.v0, __b.v0);
  ret.v1 = _mm_andnot_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_or_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_or_pd(__a.v0, __b.v0);
  ret.v1 = _mm_or_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_or_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_or_ps(__a.v0, __b.v0);
  ret.v1 = _mm_or_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_xor_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_xor_pd(__a.v0, __b.v0);
  ret.v1 = _mm_xor_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_xor_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_xor_ps(__a.v0, __b.v0);
  ret.v1 = _mm_xor_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_hadd_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_hadd_pd(__a.v0, __b.v0);
  ret.v1 = _mm_hadd_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_hadd_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_hadd_ps(__a.v0, __b.v0);
  ret.v1 = _mm_hadd_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_hsub_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_hsub_pd(__a.v0, __b.v0);
  ret.v1 = _mm_hsub_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_hsub_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_hsub_ps(__a.v0, __b.v0);
  ret.v1 = _mm_hsub_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_permutevar_pd(__m128d __a, __m128i __c) {
  return (__m128d)wasm_f64x2_make(
    ((__f64x2)__a)[(wasm_i64x2_extract_lane(__c, 0) >> 1) & 1],
    ((__f64x2)__a)[(wasm_i64x2_extract_lane(__c, 1) >> 1) & 1]);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_permutevar_pd(__m256d __a, __m256i __c) {
  __m256d ret;
  ret.v0 = _mm_permutevar_pd(__a.v0, __c.v0);
  ret.v1 = _mm_permutevar_pd(__a.v1, __c.v1);
  return ret;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_permutevar_ps(__m128 __a, __m128i __c) {
  return (__m128)wasm_f32x4_make(
    ((__f32x4)__a)[wasm_i32x4_extract_lane(__c, 0) & 3],
    ((__f32x4)__a)[wasm_i32x4_extract_lane(__c, 1) & 3],
    ((__f32x4)__a)[wasm_i32x4_extract_lane(__c, 2) & 3],
    ((__f32x4)__a)[wasm_i32x4_extract_lane(__c, 3) & 3]);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_permutevar_ps(__m256 __a, __m256i __c) {
  __m256 ret;
  ret.v0 = _mm_permutevar_ps(__a.v0, __c.v0);
  ret.v1 = _mm_permutevar_ps(__a.v1, __c.v1);
  return ret;
}

#define _mm_permute_pd(__a, __imm)                                             \
  ((__m128d)wasm_i64x2_shuffle(                                                \
    (__m128d)(__a), (__m128d)(__a), ((__imm) & 1), (((__imm) >> 1) & 1)))

#define _mm256_permute_pd(__A, __imm)                                          \
  __extension__({                                                              \
    __m256d __a = (__A);                                                       \
    _mm256_set_m128d(_mm_permute_pd(__a.v1, (__imm) >> 2),                     \
                     _mm_permute_pd(__a.v0, (__imm)));                         \
  })

#define _mm_permute_ps(__a, __imm)                                             \
  ((__m128)wasm_i32x4_shuffle((__m128)(__a),                                   \
                              (__m128)(__a),                                   \
                              ((__imm) & 3),                                   \
                              (((__imm) >> 2) & 3),                            \
                              (((__imm) >> 4) & 3),                            \
                              (((__imm) >> 6) & 3)))

#define _mm256_permute_ps(__A, __imm)                                          \
  __extension__({                                                              \
    __m256 __a = (__A);                                                        \
    _mm256_set_m128(_mm_permute_ps(__a.v1, (__imm)),                           \
                    _mm_permute_ps(__a.v0, (__imm)));                          \
  })

static __inline__ __m128d
__avx_select4d(__m256d __a, __m256d __b, const int imm8) {
  switch (imm8 & 0xF) {
    case 0:
    case 4:
      return __a.v0;
    case 1:
    case 5:
      return __a.v1;
    case 2:
    case 6:
      return __b.v0;
    case 3:
    case 7:
      return __b.v1;
    default:
      return (__m128d)wasm_i64x2_const_splat(0);
  }
}

static __inline__ __m128 __avx_select4(__m256 __a, __m256 __b, const int imm8) {
  switch (imm8 & 0xF) {
    case 0:
    case 4:
      return __a.v0;
    case 1:
    case 5:
      return __a.v1;
    case 2:
    case 6:
      return __b.v0;
    case 3:
    case 7:
      return __b.v1;
    default:
      return (__m128)wasm_i64x2_const_splat(0);
  }
}

static __inline__ __m128i
__avx_select4i(__m256i __a, __m256i __b, const int imm8) {
  switch (imm8 & 0xF) {
    case 0:
    case 4:
      return __a.v0;
    case 1:
    case 5:
      return __a.v1;
    case 2:
    case 6:
      return __b.v0;
    case 3:
    case 7:
      return __b.v1;
    default:
      return wasm_i64x2_const_splat(0);
  }
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_permute2f128_pd(__m256d __a, __m256d __b, const int imm8) {
  __m256d ret;
  ret.v0 = __avx_select4d(__a, __b, imm8);
  ret.v1 = __avx_select4d(__a, __b, imm8 >> 4);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_permute2f128_ps(__m256 __a, __m256 __b, const int imm8) {
  __m256 ret;
  ret.v0 = __avx_select4(__a, __b, imm8);
  ret.v1 = __avx_select4(__a, __b, imm8 >> 4);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_permute2f128_si256(__m256i __a, __m256i __b, const int imm8) {
  __m256i ret;
  ret.v0 = __avx_select4i(__a, __b, imm8);
  ret.v1 = __avx_select4i(__a, __b, imm8 >> 4);
  return ret;
}

#define _mm256_blend_pd(__A, __B, imm8)                                        \
  __extension__({                                                              \
    __m256d __a = (__A);                                                       \
    __m256d __b = (__B);                                                       \
    _mm256_set_m128d(_mm_blend_pd(__a.v1, __b.v1, (imm8) >> 2),                \
                     _mm_blend_pd(__a.v0, __b.v0, (imm8)));                    \
  })

#define _mm256_blend_ps(__A, __B, imm)                                         \
  __extension__({                                                              \
    __m256 __a = (__A);                                                        \
    __m256 __b = (__B);                                                        \
    _mm256_set_m128(_mm_blend_ps(__a.v1, __b.v1, (imm) >> 4),                  \
                    _mm_blend_ps(__a.v0, __b.v0, (imm)));                      \
  })

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_blendv_pd(__m256d __a, __m256d __b, __m256d __c) {
  __m256d ret;
  ret.v0 = _mm_blendv_pd(__a.v0, __b.v0, __c.v0);
  ret.v1 = _mm_blendv_pd(__a.v1, __b.v1, __c.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_blendv_ps(__m256 __a, __m256 __b, __m256 __c) {
  __m256 ret;
  ret.v0 = _mm_blendv_ps(__a.v0, __b.v0, __c.v0);
  ret.v1 = _mm_blendv_ps(__a.v1, __b.v1, __c.v1);
  return ret;
}

#define _mm256_dp_ps(__A, __B, imm)                                            \
  __extension__({                                                              \
    __m256 __a = (__A);                                                        \
    __m256 __b = (__B);                                                        \
    _mm256_set_m128(_mm_dp_ps(__a.v1, __b.v1, (imm)),                          \
                    _mm_dp_ps(__a.v0, __b.v0, (imm)));                         \
  })

#define _mm256_shuffle_ps(__A, __B, mask)                                      \
  __extension__({                                                              \
    __m256 __a = (__A);                                                        \
    __m256 __b = (__B);                                                        \
    _mm256_set_m128(_mm_shuffle_ps(__a.v1, __b.v1, (mask)),                    \
                    _mm_shuffle_ps(__a.v0, __b.v0, (mask)));                   \
  })

#define _mm256_shuffle_pd(__A, __B, mask)                                      \
  __extension__({                                                              \
    __m256d __a = (__A);                                                       \
    __m256d __b = (__B);                                                       \
    _mm256_set_m128d(_mm_shuffle_pd(__a.v1, __b.v1, (mask) >> 2),              \
                     _mm_shuffle_pd(__a.v0, __b.v0, (mask)));                  \
  })

#define _CMP_EQ_OQ 0
#define _CMP_LT_OS 1
#define _CMP_LE_OS 2
#define _CMP_UNORD_Q 3
#define _CMP_NEQ_UQ 4
#define _CMP_NLT_US 5
#define _CMP_NLE_US 6
#define _CMP_ORD_Q 7
#define _CMP_EQ_UQ 8
#define _CMP_NGE_US 9
#define _CMP_NGT_US 10
#define _CMP_FALSE_OQ 11
#define _CMP_NEQ_OQ 12
#define _CMP_GE_OS 13
#define _CMP_GT_OS 14
#define _CMP_TRUE_UQ 15
#define _CMP_EQ_OS 16
#define _CMP_LT_OQ 17
#define _CMP_LE_OQ 18
#define _CMP_UNORD_S 19
#define _CMP_NEQ_US 20
#define _CMP_NLT_UQ 21
#define _CMP_NLE_UQ 22
#define _CMP_ORD_S 23
#define _CMP_EQ_US 24
#define _CMP_NGE_UQ 25
#define _CMP_NGT_UQ 26
#define _CMP_FALSE_OS 27
#define _CMP_NEQ_OS 28
#define _CMP_GE_OQ 29
#define _CMP_GT_OQ 30
#define _CMP_TRUE_US 31

#define _mm_cmp_pd(__a, __b, __imm)                                            \
  __extension__({                                                              \
    __m128d __ret;                                                             \
    switch ((__imm)) {                                                         \
      case _CMP_EQ_OQ:                                                         \
      case _CMP_EQ_OS:                                                         \
        __ret = _mm_cmpeq_pd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_EQ_UQ:                                                         \
      case _CMP_EQ_US:                                                         \
        __ret = _mm_or_pd(_mm_cmpeq_pd((__a), (__b)),                          \
                          _mm_cmpunord_pd((__a), (__b)));                      \
        break;                                                                 \
      case _CMP_LT_OS:                                                         \
      case _CMP_LT_OQ:                                                         \
        __ret = _mm_cmplt_pd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_LE_OS:                                                         \
      case _CMP_LE_OQ:                                                         \
        __ret = _mm_cmple_pd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_UNORD_Q:                                                       \
      case _CMP_UNORD_S:                                                       \
        __ret = _mm_cmpunord_pd((__a), (__b));                                 \
        break;                                                                 \
      case _CMP_NEQ_UQ:                                                        \
      case _CMP_NEQ_US:                                                        \
        __ret = _mm_cmpneq_pd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NEQ_OQ:                                                        \
      case _CMP_NEQ_OS:                                                        \
        __ret = _mm_andnot_pd(_mm_cmpunord_pd((__a), (__b)),                   \
                              _mm_cmpneq_pd((__a), (__b)));                    \
        break;                                                                 \
      case _CMP_NLT_US:                                                        \
      case _CMP_NLT_UQ:                                                        \
        __ret = _mm_cmpnlt_pd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_ORD_Q:                                                         \
      case _CMP_ORD_S:                                                         \
        __ret = _mm_cmpord_pd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGE_US:                                                        \
      case _CMP_NGE_UQ:                                                        \
        __ret = _mm_cmpnge_pd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGT_US:                                                        \
      case _CMP_NGT_UQ:                                                        \
        __ret = _mm_cmpngt_pd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_FALSE_OQ:                                                      \
      case _CMP_FALSE_OS:                                                      \
        __ret = _mm_setzero_pd();                                              \
        break;                                                                 \
      case _CMP_GE_OS:                                                         \
      case _CMP_GE_OQ:                                                         \
        __ret = _mm_cmpge_pd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_GT_OS:                                                         \
      case _CMP_GT_OQ:                                                         \
        __ret = _mm_cmpgt_pd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_TRUE_UQ:                                                       \
      case _CMP_TRUE_US:                                                       \
        __ret = (__m128d)wasm_i8x16_splat(0xFF);                               \
        break;                                                                 \
      case _CMP_NLE_US:                                                        \
      case _CMP_NLE_UQ:                                                        \
        __ret = _mm_cmpnle_pd((__a), (__b));                                   \
        break;                                                                 \
    }                                                                          \
    __ret;                                                                     \
  })

#define _mm_cmp_ps(__a, __b, __imm)                                            \
  __extension__({                                                              \
    __m128 __ret;                                                              \
    switch ((__imm)) {                                                         \
      case _CMP_EQ_OQ:                                                         \
      case _CMP_EQ_OS:                                                         \
        __ret = _mm_cmpeq_ps((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_EQ_UQ:                                                         \
      case _CMP_EQ_US:                                                         \
        __ret = _mm_or_ps(_mm_cmpeq_ps((__a), (__b)),                          \
                          _mm_cmpunord_ps((__a), (__b)));                      \
        break;                                                                 \
      case _CMP_LT_OS:                                                         \
      case _CMP_LT_OQ:                                                         \
        __ret = _mm_cmplt_ps((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_LE_OS:                                                         \
      case _CMP_LE_OQ:                                                         \
        __ret = _mm_cmple_ps((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_UNORD_Q:                                                       \
      case _CMP_UNORD_S:                                                       \
        __ret = _mm_cmpunord_ps((__a), (__b));                                 \
        break;                                                                 \
      case _CMP_NEQ_UQ:                                                        \
      case _CMP_NEQ_US:                                                        \
        __ret = _mm_cmpneq_ps((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NEQ_OQ:                                                        \
      case _CMP_NEQ_OS:                                                        \
        __ret = _mm_andnot_ps(_mm_cmpunord_ps((__a), (__b)),                   \
                              _mm_cmpneq_ps((__a), (__b)));                    \
        break;                                                                 \
      case _CMP_NLT_US:                                                        \
      case _CMP_NLT_UQ:                                                        \
        __ret = _mm_cmpnlt_ps((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_ORD_Q:                                                         \
      case _CMP_ORD_S:                                                         \
        __ret = _mm_cmpord_ps((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGE_US:                                                        \
      case _CMP_NGE_UQ:                                                        \
        __ret = _mm_cmpnge_ps((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGT_US:                                                        \
      case _CMP_NGT_UQ:                                                        \
        __ret = _mm_cmpngt_ps((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_FALSE_OQ:                                                      \
      case _CMP_FALSE_OS:                                                      \
        __ret = _mm_setzero_ps();                                              \
        break;                                                                 \
      case _CMP_GE_OS:                                                         \
      case _CMP_GE_OQ:                                                         \
        __ret = _mm_cmpge_ps((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_GT_OS:                                                         \
      case _CMP_GT_OQ:                                                         \
        __ret = _mm_cmpgt_ps((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_TRUE_UQ:                                                       \
      case _CMP_TRUE_US:                                                       \
        __ret = (__m128)wasm_i8x16_splat(0xFF);                                \
        break;                                                                 \
      case _CMP_NLE_US:                                                        \
      case _CMP_NLE_UQ:                                                        \
        __ret = _mm_cmpnle_ps((__a), (__b));                                   \
        break;                                                                 \
    }                                                                          \
    __ret;                                                                     \
  })

#define _mm_cmp_sd(__a, __b, __imm)                                            \
  __extension__({                                                              \
    __m128d __ret;                                                             \
    switch ((__imm)) {                                                         \
      case _CMP_EQ_OQ:                                                         \
      case _CMP_EQ_OS:                                                         \
        __ret = _mm_cmpeq_sd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_EQ_UQ:                                                         \
      case _CMP_EQ_US:                                                         \
        __ret = _mm_move_sd((__a),                                             \
                            _mm_or_pd(_mm_cmpeq_sd((__a), (__b)),              \
                                      _mm_cmpunord_sd((__a), (__b))));         \
        break;                                                                 \
      case _CMP_LT_OS:                                                         \
      case _CMP_LT_OQ:                                                         \
        __ret = _mm_cmplt_sd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_LE_OS:                                                         \
      case _CMP_LE_OQ:                                                         \
        __ret = _mm_cmple_sd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_UNORD_Q:                                                       \
      case _CMP_UNORD_S:                                                       \
        __ret = _mm_cmpunord_sd((__a), (__b));                                 \
        break;                                                                 \
      case _CMP_NEQ_UQ:                                                        \
      case _CMP_NEQ_US:                                                        \
        __ret = _mm_cmpneq_sd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NEQ_OQ:                                                        \
      case _CMP_NEQ_OS:                                                        \
        __ret = _mm_move_sd((__a),                                             \
                            _mm_andnot_pd(_mm_cmpunord_sd((__a), (__b)),       \
                                          _mm_cmpneq_sd((__a), (__b))));       \
        break;                                                                 \
      case _CMP_NLT_US:                                                        \
      case _CMP_NLT_UQ:                                                        \
        __ret = _mm_cmpnlt_sd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_ORD_Q:                                                         \
      case _CMP_ORD_S:                                                         \
        __ret = _mm_cmpord_sd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGE_US:                                                        \
      case _CMP_NGE_UQ:                                                        \
        __ret = _mm_cmpnge_sd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGT_US:                                                        \
      case _CMP_NGT_UQ:                                                        \
        __ret = _mm_cmpngt_sd((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_FALSE_OQ:                                                      \
      case _CMP_FALSE_OS:                                                      \
        __ret = _mm_move_sd((__a), _mm_setzero_pd());                          \
        break;                                                                 \
      case _CMP_GE_OS:                                                         \
      case _CMP_GE_OQ:                                                         \
        __ret = _mm_cmpge_sd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_GT_OS:                                                         \
      case _CMP_GT_OQ:                                                         \
        __ret = _mm_cmpgt_sd((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_TRUE_UQ:                                                       \
      case _CMP_TRUE_US:                                                       \
        __ret = _mm_move_sd((__a), (__m128d)wasm_i8x16_splat(0xFF));           \
        break;                                                                 \
      case _CMP_NLE_US:                                                        \
      case _CMP_NLE_UQ:                                                        \
        __ret = _mm_cmpnle_sd((__a), (__b));                                   \
        break;                                                                 \
    }                                                                          \
    __ret;                                                                     \
  })

#define _mm_cmp_ss(__a, __b, __imm)                                            \
  __extension__({                                                              \
    __m128 __ret;                                                              \
    switch ((__imm)) {                                                         \
      case _CMP_EQ_OQ:                                                         \
      case _CMP_EQ_OS:                                                         \
        __ret = _mm_cmpeq_ss((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_EQ_UQ:                                                         \
      case _CMP_EQ_US:                                                         \
        __ret = _mm_move_ss((__a),                                             \
                            _mm_or_ps(_mm_cmpeq_ss((__a), (__b)),              \
                                      _mm_cmpunord_ss((__a), (__b))));         \
        break;                                                                 \
      case _CMP_LT_OS:                                                         \
      case _CMP_LT_OQ:                                                         \
        __ret = _mm_cmplt_ss((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_LE_OS:                                                         \
      case _CMP_LE_OQ:                                                         \
        __ret = _mm_cmple_ss((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_UNORD_Q:                                                       \
      case _CMP_UNORD_S:                                                       \
        __ret = _mm_cmpunord_ss((__a), (__b));                                 \
        break;                                                                 \
      case _CMP_NEQ_UQ:                                                        \
      case _CMP_NEQ_US:                                                        \
        __ret = _mm_cmpneq_ss((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NEQ_OQ:                                                        \
      case _CMP_NEQ_OS:                                                        \
        __ret = _mm_move_ss((__a),                                             \
                            _mm_andnot_ps(_mm_cmpunord_ss((__a), (__b)),       \
                                          _mm_cmpneq_ss((__a), (__b))));       \
        break;                                                                 \
      case _CMP_NLT_US:                                                        \
      case _CMP_NLT_UQ:                                                        \
        __ret = _mm_cmpnlt_ss((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_ORD_Q:                                                         \
      case _CMP_ORD_S:                                                         \
        __ret = _mm_cmpord_ss((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGE_US:                                                        \
      case _CMP_NGE_UQ:                                                        \
        __ret = _mm_cmpnge_ss((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_NGT_US:                                                        \
      case _CMP_NGT_UQ:                                                        \
        __ret = _mm_cmpngt_ss((__a), (__b));                                   \
        break;                                                                 \
      case _CMP_FALSE_OQ:                                                      \
      case _CMP_FALSE_OS:                                                      \
        __ret = _mm_move_ss((__a), _mm_setzero_ps());                          \
        break;                                                                 \
      case _CMP_GE_OS:                                                         \
      case _CMP_GE_OQ:                                                         \
        __ret = _mm_cmpge_ss((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_GT_OS:                                                         \
      case _CMP_GT_OQ:                                                         \
        __ret = _mm_cmpgt_ss((__a), (__b));                                    \
        break;                                                                 \
      case _CMP_TRUE_UQ:                                                       \
      case _CMP_TRUE_US:                                                       \
        __ret = _mm_move_ss((__a), (__m128)wasm_i8x16_splat(0xFF));            \
        break;                                                                 \
      case _CMP_NLE_US:                                                        \
      case _CMP_NLE_UQ:                                                        \
        __ret = _mm_cmpnle_ss((__a), (__b));                                   \
        break;                                                                 \
    }                                                                          \
    __ret;                                                                     \
  })

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_cmp_pd(__m256d a, __m256d b, const int imm8) {
  __m256d ret;
  ret.v0 = _mm_cmp_pd(a.v0, b.v0, imm8);
  ret.v1 = _mm_cmp_pd(a.v1, b.v1, imm8);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_cmp_ps(__m256 __a, __m256 __b, const int imm8) {
  __m256 ret;
  ret.v0 = _mm_cmp_ps(__a.v0, __b.v0, imm8);
  ret.v1 = _mm_cmp_ps(__a.v1, __b.v1, imm8);
  return ret;
}

#define _mm256_extract_epi32(__A, N)                                           \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    ((N) & 0x7) < 4 ? _mm_extract_epi32(__a.v0, (N) & 0x3)                     \
                    : _mm_extract_epi32(__a.v1, (N) & 0x3);                    \
  })

#define _mm256_extract_epi16(__A, N)                                           \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    ((N) & 0xF) < 8 ? _mm_extract_epi16(__a.v0, (N) & 0x7)                     \
                    : _mm_extract_epi16(__a.v1, (N) & 0x7);                    \
  })

#define _mm256_extract_epi8(__A, N)                                            \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    ((N) & 0x1F) < 16 ? _mm_extract_epi8(__a.v0, (N) & 0xF)                    \
                      : _mm_extract_epi8(__a.v1, (N) & 0xF);                   \
  })

#define _mm256_extract_epi64(__A, N)                                           \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    ((N) & 0x3) < 2 ? _mm_extract_epi64(__a.v0, (N) & 0x1)                     \
                    : _mm_extract_epi64(__a.v1, (N) & 0x1);                    \
  })

#define _mm256_insert_epi32(__A, __I, N)                                       \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    int32_t __i = (__I);                                                       \
    ((N) & 0x7) < 4                                                            \
      ? _mm256_set_m128i(__a.v1, _mm_insert_epi32(__a.v0, __i, (N) & 0x3))     \
      : _mm256_set_m128i(_mm_insert_epi32(__a.v1, __i, (N) & 0x3), __a.v0);    \
  })

#define _mm256_insert_epi16(__A, __I, N)                                       \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    int16_t __i = (__I);                                                       \
    ((N) & 0xF) < 8                                                            \
      ? _mm256_set_m128i(__a.v1, _mm_insert_epi16(__a.v0, __i, (N) & 0x7))     \
      : _mm256_set_m128i(_mm_insert_epi16(__a.v1, __i, (N) & 0x7), __a.v0);    \
  })

#define _mm256_insert_epi8(__A, __I, N)                                        \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    int8_t __i = (__I);                                                        \
    ((N) & 0x1F) < 16                                                          \
      ? _mm256_set_m128i(__a.v1, _mm_insert_epi8(__a.v0, __i, (N) & 0xF))      \
      : _mm256_set_m128i(_mm_insert_epi8(__a.v1, __i, (N) & 0xF), __a.v0);     \
  })

#define _mm256_insert_epi64(__A, __I, N)                                       \
  __extension__({                                                              \
    __m256i __a = (__A);                                                       \
    int64_t __i = (__I);                                                       \
    ((N) & 0x3) < 2                                                            \
      ? _mm256_set_m128i(__a.v1, _mm_insert_epi64(__a.v0, __i, (N) & 0x1))     \
      : _mm256_set_m128i(_mm_insert_epi64(__a.v1, __i, (N) & 0x1), __a.v0);    \
  })

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi32_pd(__m128i __a) {
  __m256d ret;
  ret.v0 = _mm_cvtepi32_pd(__a);
  __m128i __a1 = wasm_i32x4_shuffle(__a, __a, 2, 3, 0, 0);
  ret.v1 = _mm_cvtepi32_pd(__a1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_cvtepi32_ps(__m256i __a) {
  __m256 ret;
  ret.v0 = _mm_cvtepi32_ps(__a.v0);
  ret.v1 = _mm_cvtepi32_ps(__a.v1);
  return ret;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm256_cvtpd_ps(__m256d __a) {
  __m128 low = _mm_cvtpd_ps(__a.v0);
  __m128 high = _mm_cvtpd_ps(__a.v1);
  __m128 ret = (__m128)wasm_i32x4_shuffle(low, high, 0, 1, 4, 5);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtps_epi32(__m256 __a) {
  __m256i ret;
  ret.v0 = _mm_cvtps_epi32(__a.v0);
  ret.v1 = _mm_cvtps_epi32(__a.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_cvtps_pd(__m128 __a) {
  __m256d ret;
  ret.v0 = _mm_cvtps_pd(__a);
  __m128 __a1 = (__m128)wasm_i32x4_shuffle(__a, __a, 2, 3, 0, 0);
  ret.v1 = _mm_cvtps_pd(__a1);
  return ret;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm256_cvttpd_epi32(__m256d __a) {
  __m128i low = _mm_cvttpd_epi32(__a.v0);
  __m128i high = _mm_cvttpd_epi32(__a.v1);
  __m128i ret = wasm_i32x4_shuffle(low, high, 0, 1, 4, 5);
  return ret;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm256_cvtpd_epi32(__m256d __a) {
  __m128i low = _mm_cvtpd_epi32(__a.v0);
  __m128i high = _mm_cvtpd_epi32(__a.v1);
  __m128i ret = wasm_i32x4_shuffle(low, high, 0, 1, 4, 5);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_cvttps_epi32(__m256 __a) {
  __m256i ret;
  ret.v0 = _mm_cvttps_epi32(__a.v0);
  ret.v1 = _mm_cvttps_epi32(__a.v1);
  return ret;
}

static __inline__ double __attribute__((__always_inline__, __nodebug__))
_mm256_cvtsd_f64(__m256d __a) {
  return _mm_cvtsd_f64(__a.v0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_cvtsi256_si32(__m256i __a) {
  return _mm_cvtsi128_si32(__a.v0);
}

static __inline__ float __attribute__((__always_inline__, __nodebug__))
_mm256_cvtss_f32(__m256 __a) {
  return _mm_cvtss_f32(__a.v0);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_movehdup_ps(__m256 __a) {
  __m256 ret;
  ret.v0 = _mm_movehdup_ps(__a.v0);
  ret.v1 = _mm_movehdup_ps(__a.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_moveldup_ps(__m256 __a) {
  __m256 ret;
  ret.v0 = _mm_moveldup_ps(__a.v0);
  ret.v1 = _mm_moveldup_ps(__a.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_movedup_pd(__m256d __a) {
  __m256d ret;
  ret.v0 = _mm_movedup_pd(__a.v0);
  ret.v1 = _mm_movedup_pd(__a.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_unpackhi_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_unpackhi_pd(__a.v0, __b.v0);
  ret.v1 = _mm_unpackhi_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_unpacklo_pd(__m256d __a, __m256d __b) {
  __m256d ret;
  ret.v0 = _mm_unpacklo_pd(__a.v0, __b.v0);
  ret.v1 = _mm_unpacklo_pd(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_unpackhi_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_unpackhi_ps(__a.v0, __b.v0);
  ret.v1 = _mm_unpackhi_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_unpacklo_ps(__m256 __a, __m256 __b) {
  __m256 ret;
  ret.v0 = _mm_unpacklo_ps(__a.v0, __b.v0);
  ret.v1 = _mm_unpacklo_ps(__a.v1, __b.v1);
  return ret;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testz_pd(__m128d __a, __m128d __b) {
  v128_t __m =
    wasm_u64x2_shr(wasm_v128_not(wasm_v128_and((v128_t)__a, (v128_t)__b)), 63);
  return wasm_i64x2_extract_lane(__m, 0) & wasm_i64x2_extract_lane(__m, 1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testc_pd(__m128d __a, __m128d __b) {
  v128_t __m =
    wasm_u64x2_shr(wasm_v128_or(wasm_v128_not((v128_t)__b), (v128_t)__a), 63);
  return wasm_i64x2_extract_lane(__m, 0) & wasm_i64x2_extract_lane(__m, 1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testnzc_pd(__m128d __a, __m128d __b) {
  v128_t __m = wasm_u64x2_shr(wasm_v128_and((v128_t)__a, (v128_t)__b), 63);
  v128_t __m2 = wasm_u64x2_shr(wasm_v128_andnot((v128_t)__b, (v128_t)__a), 63);
  return (wasm_i64x2_extract_lane(__m, 0) | wasm_i64x2_extract_lane(__m, 1)) &
         (wasm_i64x2_extract_lane(__m2, 0) | wasm_i64x2_extract_lane(__m2, 1));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testz_ps(__m128 __a, __m128 __b) {
  v128_t __m =
    wasm_u32x4_shr(wasm_v128_not(wasm_v128_and((v128_t)__a, (v128_t)__b)), 31);
  __m = wasm_v128_and(__m, (v128_t)_mm_movehl_ps((__m128)__m, (__m128)__m));
  __m = wasm_v128_and(__m, _mm_shuffle_epi32(__m, _MM_SHUFFLE(3, 2, 0, 1)));
  return wasm_i32x4_extract_lane(__m, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testc_ps(__m128 __a, __m128 __b) {
  v128_t __m =
    wasm_u32x4_shr(wasm_v128_or(wasm_v128_not((v128_t)__b), (v128_t)__a), 31);
  __m = wasm_v128_and(__m, (v128_t)_mm_movehl_ps((__m128)__m, (__m128)__m));
  __m = wasm_v128_and(__m, _mm_shuffle_epi32(__m, _MM_SHUFFLE(3, 2, 0, 1)));
  return wasm_i32x4_extract_lane(__m, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testnzc_ps(__m128 __a, __m128 __b) {
  v128_t __m = wasm_u32x4_shr(wasm_v128_and((v128_t)__a, (v128_t)__b), 31);
  v128_t __m2 = wasm_u32x4_shr(wasm_v128_andnot((v128_t)__b, (v128_t)__a), 31);

  __m = wasm_v128_or(__m, (v128_t)_mm_movehl_ps((__m128)__m, (__m128)__m));
  __m2 = wasm_v128_or(__m2, (v128_t)_mm_movehl_ps((__m128)__m2, (__m128)__m2));
  __m = wasm_v128_or(__m, _mm_shuffle_epi32(__m, _MM_SHUFFLE(3, 2, 0, 1)));
  __m2 = wasm_v128_or(__m2, _mm_shuffle_epi32(__m2, _MM_SHUFFLE(3, 2, 0, 1)));

  return wasm_i32x4_extract_lane(__m, 0) & wasm_i32x4_extract_lane(__m2, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testz_pd(__m256d __a, __m256d __b) {
  return _mm_testz_pd(__a.v0, __b.v0) & _mm_testz_pd(__a.v1, __b.v1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testc_pd(__m256d __a, __m256d __b) {
  return _mm_testc_pd(__a.v0, __b.v0) & _mm_testc_pd(__a.v1, __b.v1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testnzc_pd(__m256d __a, __m256d __b) {
  v128_t __m =
    wasm_u64x2_shr(wasm_v128_and((v128_t)__a.v0, (v128_t)__b.v0), 63);
  v128_t __m1 =
    wasm_u64x2_shr(wasm_v128_and((v128_t)__a.v1, (v128_t)__b.v1), 63);
  v128_t __m2 =
    wasm_u64x2_shr(wasm_v128_andnot((v128_t)__b.v0, (v128_t)__a.v0), 63);
  v128_t __m3 =
    wasm_u64x2_shr(wasm_v128_andnot((v128_t)__b.v1, (v128_t)__a.v1), 63);
  return wasm_v128_any_true(wasm_v128_or(__m, __m1)) &
         wasm_v128_any_true(wasm_v128_or(__m2, __m3));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testz_ps(__m256 __a, __m256 __b) {
  return _mm_testz_ps(__a.v0, __b.v0) & _mm_testz_ps(__a.v1, __b.v1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testc_ps(__m256 __a, __m256 __b) {
  return _mm_testc_ps(__a.v0, __b.v0) & _mm_testc_ps(__a.v1, __b.v1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testnzc_ps(__m256 __a, __m256 __b) {
  v128_t __m =
    wasm_u32x4_shr(wasm_v128_and((v128_t)__a.v0, (v128_t)__b.v0), 31);
  v128_t __m1 =
    wasm_u32x4_shr(wasm_v128_and((v128_t)__a.v1, (v128_t)__b.v1), 31);
  v128_t __m2 =
    wasm_u32x4_shr(wasm_v128_andnot((v128_t)__b.v0, (v128_t)__a.v0), 31);
  v128_t __m3 =
    wasm_u32x4_shr(wasm_v128_andnot((v128_t)__b.v1, (v128_t)__a.v1), 31);

  return wasm_v128_any_true(wasm_v128_or(__m, __m1)) &
         wasm_v128_any_true(wasm_v128_or(__m2, __m3));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testz_si256(__m256i __a, __m256i __b) {
  return _mm_testz_si128(__a.v0, __b.v0) & _mm_testz_si128(__a.v1, __b.v1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testc_si256(__m256i __a, __m256i __b) {
  return _mm_testc_si128(__a.v0, __b.v0) & _mm_testc_si128(__a.v1, __b.v1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_testnzc_si256(__m256i __a, __m256i __b) {
  v128_t __m = wasm_v128_and(__a.v0, __b.v0);
  v128_t __m1 = wasm_v128_and(__a.v1, __b.v1);
  v128_t __m2 = wasm_v128_andnot(__b.v0, __a.v0);
  v128_t __m3 = wasm_v128_andnot(__b.v1, __a.v1);
  return wasm_v128_any_true(wasm_v128_or(__m, __m1)) &
         wasm_v128_any_true(wasm_v128_or(__m2, __m3));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_movemask_pd(__m256d __a) {
  return _mm_movemask_pd(__a.v0) | (_mm_movemask_pd(__a.v1) << 2);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm256_movemask_ps(__m256 __a) {
  return _mm_movemask_ps(__a.v0) | (_mm_movemask_ps(__a.v1) << 4);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_zeroall(void) {
  // Do nothing
  // when porting any assembly code that would have calls to these functions
  // around, that assembly code in the first place will not compile.
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_zeroupper(void) {
  // Do nothing
  // when porting any assembly code that would have calls to these functions
  // around, that assembly code in the first place will not compile.
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_broadcast_ss(float const* __a) {
  return (__m128)wasm_v128_load32_splat(__a);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_broadcast_sd(double const* __a) {
  __m256d ret;
  ret.v1 = ret.v0 = (__m128d)wasm_v128_load64_splat(__a);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_broadcast_ss(float const* __a) {
  __m256 ret;
  ret.v1 = ret.v0 = _mm_broadcast_ss(__a);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_broadcast_pd(__m128d const* __a) {
  __m256d ret;
  ret.v1 = ret.v0 = (__m128d)wasm_v128_load(__a);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_broadcast_ps(__m128 const* __a) {
  __m256 ret;
  ret.v1 = ret.v0 = (__m128)wasm_v128_load(__a);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_load_pd(double const* __p) {
  __m256d ret;
  ret.v0 = _mm_load_pd(__p);
  ret.v1 = _mm_load_pd(__p + 2);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_load_ps(float const* __p) {
  __m256 ret;
  ret.v0 = _mm_load_ps(__p);
  ret.v1 = _mm_load_ps(__p + 4);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_loadu_pd(double const* __p) {
  __m256d ret;
  ret.v0 = _mm_loadu_pd(__p);
  ret.v1 = _mm_loadu_pd(__p + 2);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_loadu_ps(float const* __p) {
  __m256 ret;
  ret.v0 = _mm_loadu_ps(__p);
  ret.v1 = _mm_loadu_ps(__p + 4);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_load_si256(__m256i const* __p) {
  __m256i ret;
  ret.v0 = _mm_load_si128((__m128i const*)__p);
  ret.v1 = _mm_load_si128(((__m128i const*)__p) + 1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_loadu_si256(__m256i_u const* __p) {
  __m256i ret;
  ret.v0 = _mm_loadu_si128((__m128i const*)__p);
  ret.v1 = _mm_loadu_si128(((__m128i const*)__p) + 1);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_lddqu_si256(__m256i_u const* __p) {
  __m256i ret;
  ret.v0 = _mm_lddqu_si128((__m128i const*)__p);
  ret.v1 = _mm_lddqu_si128(((__m128i const*)__p) + 1);
  return ret;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_store_pd(double* __p, __m256d __a) {
  _mm_store_pd(__p, __a.v0);
  _mm_store_pd(__p + 2, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_store_ps(float* __p, __m256 __a) {
  _mm_store_ps(__p, __a.v0);
  _mm_store_ps(__p + 4, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_storeu_pd(double* __p, __m256d __a) {
  _mm_storeu_pd(__p, __a.v0);
  _mm_storeu_pd(__p + 2, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_storeu_ps(float* __p, __m256 __a) {
  _mm_storeu_ps(__p, __a.v0);
  _mm_storeu_ps(__p + 4, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_store_si256(__m256i* __p, __m256i __a) {
  _mm_store_si128((__m128i*)__p, __a.v0);
  _mm_store_si128(((__m128i*)__p) + 1, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_storeu_si256(__m256i_u* __p, __m256i __a) {
  _mm_storeu_si128((__m128i*)__p, __a.v0);
  _mm_storeu_si128(((__m128i*)__p) + 1, __a.v1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_maskload_pd(double const* __p, __m128i __m) {
  // This may cause an out-of-bounds memory load since we first load and
  // then mask, but since there are no segmentation faults in Wasm memory
  // accesses, that is ok (as long as we are within the heap bounds -
  // a negligible limitation in practice)
  return _mm_and_pd(_mm_load_pd(__p), (__m128d)wasm_i64x2_shr(__m, 63));
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_pd(double const* __p, __m256i __m) {
  __m256d ret;
  ret.v0 = _mm_maskload_pd(__p, __m.v0);
  ret.v1 = _mm_maskload_pd(__p + 2, __m.v1);
  return ret;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_maskload_ps(float const* __p, __m128i __m) {
  // This may cause an out-of-bounds memory load since we first load and
  // then mask, but since there are no segmentation faults in Wasm memory
  // accesses, that is ok (as long as we are within the heap bounds -
  // a negligible limitation in practice)
  return _mm_and_ps(_mm_load_ps(__p), (__m128)_mm_srai_epi32(__m, 31));
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_ps(float const* __p, __m256i __m) {
  __m256 ret;
  ret.v0 = _mm_maskload_ps(__p, __m.v0);
  ret.v1 = _mm_maskload_ps(__p + 4, __m.v1);
  return ret;
}

static __inline__ void
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_maskstore_ps(float* __p, __m128i __m, __m128 __a) {
  if ((wasm_i32x4_extract_lane(__m, 0) & 0x80000000ull) != 0)
    __p[0] = wasm_f32x4_extract_lane((v128_t)__a, 0);
  if ((wasm_i32x4_extract_lane(__m, 1) & 0x80000000ull) != 0)
    __p[1] = wasm_f32x4_extract_lane((v128_t)__a, 1);
  if ((wasm_i32x4_extract_lane(__m, 2) & 0x80000000ull) != 0)
    __p[2] = wasm_f32x4_extract_lane((v128_t)__a, 2);
  if ((wasm_i32x4_extract_lane(__m, 3) & 0x80000000ull) != 0)
    __p[3] = wasm_f32x4_extract_lane((v128_t)__a, 3);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_ps(float* __p, __m256i __m, __m256 __a) {
  _mm_maskstore_ps(__p, __m.v0, __a.v0);
  _mm_maskstore_ps(__p + 4, __m.v1, __a.v1);
}

static __inline__ void
  __attribute__((__always_inline__, __nodebug__, DIAGNOSE_SLOW))
  _mm_maskstore_pd(double* __p, __m128i __m, __m128d __a) {
  if ((wasm_i64x2_extract_lane(__m, 0) & 0x8000000000000000ull) != 0)
    __p[0] = wasm_f64x2_extract_lane((v128_t)__a, 0);
  if ((wasm_i64x2_extract_lane(__m, 1) & 0x8000000000000000ull) != 0)
    __p[1] = wasm_f64x2_extract_lane((v128_t)__a, 1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_pd(double* __p, __m256i __m, __m256d __a) {
  _mm_maskstore_pd(__p, __m.v0, __a.v0);
  _mm_maskstore_pd(__p + 2, __m.v1, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_stream_si256(void* __a, __m256i __b) {
  _mm_stream_si128((__m128i*)__a, __b.v0);
  _mm_stream_si128(((__m128i*)__a) + 1, __b.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_stream_pd(void* __a, __m256d __b) {
  _mm_stream_pd((double*)__a, __b.v0);
  _mm_stream_pd(((double*)__a) + 2, __b.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_stream_ps(void* __p, __m256 __a) {
  _mm_stream_ps((float*)__p, __a.v0);
  _mm_stream_ps(((float*)__p) + 4, __a.v1);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_undefined_pd(void) {
  __m256d val;
  return val;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_undefined_ps(void) {
  __m256 val;
  return val;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_undefined_si256(void) {
  __m256i val;
  return val;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_set_pd(double __a, double __b, double __c, double __d) {
  __m256d ret;
  ret.v0 = _mm_set_pd(__c, __d);
  ret.v1 = _mm_set_pd(__a, __b);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_set_ps(float __a,
              float __b,
              float __c,
              float __d,
              float __e,
              float __f,
              float __g,
              float __h) {
  __m256 ret;
  ret.v0 = _mm_set_ps(__e, __f, __g, __h);
  ret.v1 = _mm_set_ps(__a, __b, __c, __d);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set_epi32(int __i0,
                 int __i1,
                 int __i2,
                 int __i3,
                 int __i4,
                 int __i5,
                 int __i6,
                 int __i7) {
  __m256i ret;
  ret.v0 = _mm_set_epi32(__i4, __i5, __i6, __i7);
  ret.v1 = _mm_set_epi32(__i0, __i1, __i2, __i3);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set_epi16(short __w15,
                 short __w14,
                 short __w13,
                 short __w12,
                 short __w11,
                 short __w10,
                 short __w09,
                 short __w08,
                 short __w07,
                 short __w06,
                 short __w05,
                 short __w04,
                 short __w03,
                 short __w02,
                 short __w01,
                 short __w00) {
  __m256i ret;
  ret.v0 =
    _mm_set_epi16(__w07, __w06, __w05, __w04, __w03, __w02, __w01, __w00);
  ret.v1 =
    _mm_set_epi16(__w15, __w14, __w13, __w12, __w11, __w10, __w09, __w08);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set_epi8(char __b31,
                char __b30,
                char __b29,
                char __b28,
                char __b27,
                char __b26,
                char __b25,
                char __b24,
                char __b23,
                char __b22,
                char __b21,
                char __b20,
                char __b19,
                char __b18,
                char __b17,
                char __b16,
                char __b15,
                char __b14,
                char __b13,
                char __b12,
                char __b11,
                char __b10,
                char __b09,
                char __b08,
                char __b07,
                char __b06,
                char __b05,
                char __b04,
                char __b03,
                char __b02,
                char __b01,
                char __b00) {
  __m256i ret;
  ret.v0 = _mm_set_epi8(__b15,
                        __b14,
                        __b13,
                        __b12,
                        __b11,
                        __b10,
                        __b09,
                        __b08,
                        __b07,
                        __b06,
                        __b05,
                        __b04,
                        __b03,
                        __b02,
                        __b01,
                        __b00);
  ret.v1 = _mm_set_epi8(__b31,
                        __b30,
                        __b29,
                        __b28,
                        __b27,
                        __b26,
                        __b25,
                        __b24,
                        __b23,
                        __b22,
                        __b21,
                        __b20,
                        __b19,
                        __b18,
                        __b17,
                        __b16);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set_epi64x(long long __a, long long __b, long long __c, long long __d) {
  __m256i ret;
  ret.v0 = _mm_set_epi64x(__c, __d);
  ret.v1 = _mm_set_epi64x(__a, __b);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_setr_pd(double __a, double __b, double __c, double __d) {
  return _mm256_set_pd(__d, __c, __b, __a);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_setr_ps(float __a,
               float __b,
               float __c,
               float __d,
               float __e,
               float __f,
               float __g,
               float __h) {
  return _mm256_set_ps(__h, __g, __f, __e, __d, __c, __b, __a);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_setr_epi32(int __i0,
                  int __i1,
                  int __i2,
                  int __i3,
                  int __i4,
                  int __i5,
                  int __i6,
                  int __i7) {
  return _mm256_set_epi32(__i7, __i6, __i5, __i4, __i3, __i2, __i1, __i0);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_setr_epi16(short __w15,
                  short __w14,
                  short __w13,
                  short __w12,
                  short __w11,
                  short __w10,
                  short __w09,
                  short __w08,
                  short __w07,
                  short __w06,
                  short __w05,
                  short __w04,
                  short __w03,
                  short __w02,
                  short __w01,
                  short __w00) {
  return _mm256_set_epi16(__w00,
                          __w01,
                          __w02,
                          __w03,
                          __w04,
                          __w05,
                          __w06,
                          __w07,
                          __w08,
                          __w09,
                          __w10,
                          __w11,
                          __w12,
                          __w13,
                          __w14,
                          __w15);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_setr_epi8(char __b31,
                 char __b30,
                 char __b29,
                 char __b28,
                 char __b27,
                 char __b26,
                 char __b25,
                 char __b24,
                 char __b23,
                 char __b22,
                 char __b21,
                 char __b20,
                 char __b19,
                 char __b18,
                 char __b17,
                 char __b16,
                 char __b15,
                 char __b14,
                 char __b13,
                 char __b12,
                 char __b11,
                 char __b10,
                 char __b09,
                 char __b08,
                 char __b07,
                 char __b06,
                 char __b05,
                 char __b04,
                 char __b03,
                 char __b02,
                 char __b01,
                 char __b00) {
  return _mm256_set_epi8(__b00,
                         __b01,
                         __b02,
                         __b03,
                         __b04,
                         __b05,
                         __b06,
                         __b07,
                         __b08,
                         __b09,
                         __b10,
                         __b11,
                         __b12,
                         __b13,
                         __b14,
                         __b15,
                         __b16,
                         __b17,
                         __b18,
                         __b19,
                         __b20,
                         __b21,
                         __b22,
                         __b23,
                         __b24,
                         __b25,
                         __b26,
                         __b27,
                         __b28,
                         __b29,
                         __b30,
                         __b31);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_setr_epi64x(long long __a, long long __b, long long __c, long long __d) {
  return _mm256_set_epi64x(__d, __c, __b, __a);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_set1_pd(double __w) {
  __m256d ret;
  ret.v1 = ret.v0 = (__m128d)wasm_f64x2_splat(__w);
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_set1_ps(float __w) {
  __m256 ret;
  ret.v1 = ret.v0 = (__m128)wasm_f32x4_splat(__w);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set1_epi32(int __i) {
  __m256i ret;
  ret.v1 = ret.v0 = wasm_i32x4_splat(__i);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set1_epi16(short __w) {
  __m256i ret;
  ret.v1 = ret.v0 = wasm_i16x8_splat(__w);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set1_epi8(char __b) {
  __m256i ret;
  ret.v1 = ret.v0 = wasm_i8x16_splat(__b);
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set1_epi64x(long long __q) {
  __m256i ret;
  ret.v1 = ret.v0 = wasm_i64x2_splat(__q);
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_setzero_pd(void) {
  __m256d ret;
  ret.v1 = ret.v0 = _mm_setzero_pd();
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_setzero_ps(void) {
  __m256 ret;
  ret.v1 = ret.v0 = _mm_setzero_ps();
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_setzero_si256(void) {
  __m256i ret;
  ret.v1 = ret.v0 = _mm_setzero_si128();
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_castpd_ps(__m256d __a) {
  m256_data ret;
  ret.double_view = __a;
  return ret.float_view;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_castpd_si256(__m256d __a) {
  m256_data ret;
  ret.double_view = __a;
  return ret.int_view;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_castps_pd(__m256 __a) {
  m256_data ret;
  ret.float_view = __a;
  return ret.double_view;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_castps_si256(__m256 __a) {
  m256_data ret;
  ret.float_view = __a;
  return ret.int_view;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_castsi256_ps(__m256i __a) {
  m256_data ret;
  ret.int_view = __a;
  return ret.float_view;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_castsi256_pd(__m256i __a) {
  m256_data ret;
  ret.int_view = __a;
  return ret.double_view;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm256_castpd256_pd128(__m256d __a) {
  return __a.v0;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm256_castps256_ps128(__m256 __a) {
  return __a.v0;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm256_castsi256_si128(__m256i __a) {
  return __a.v0;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_castpd128_pd256(__m128d __a) {
  __m256d ret;
  ret.v0 = __a;
  ret.v1 = _mm_setzero_pd();
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_castps128_ps256(__m128 __a) {
  __m256 ret;
  ret.v0 = __a;
  ret.v1 = _mm_setzero_ps();
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_castsi128_si256(__m128i __a) {
  __m256i ret;
  ret.v0 = __a;
  ret.v1 = _mm_setzero_si128();
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_zextpd128_pd256(__m128d __a) {
  __m256d ret;
  ret.v0 = __a;
  ret.v1 = _mm_setzero_pd();
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_zextps128_ps256(__m128 __a) {
  __m256 ret;
  ret.v0 = __a;
  ret.v1 = _mm_setzero_ps();
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_zextsi128_si256(__m128i __a) {
  __m256i ret;
  ret.v0 = __a;
  ret.v1 = _mm_setzero_si128();
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_insertf128_ps(__m256 __a, __m128 __b, const int imm8) {
  __m256 ret = __a;
  if (imm8 & 0x1) {
    ret.v1 = __b;
  } else {
    ret.v0 = __b;
  }
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_insertf128_pd(__m256d __a, __m128d __b, const int imm8) {
  __m256d ret = __a;
  if (imm8 & 0x1) {
    ret.v1 = __b;
  } else {
    ret.v0 = __b;
  }
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_insertf128_si256(__m256i __a, __m128i __b, const int imm8) {
  __m256i ret = __a;
  if (imm8 & 0x1) {
    ret.v1 = __b;
  } else {
    ret.v0 = __b;
  }
  return ret;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm256_extractf128_ps(__m256 __a, const int imm8) {
  if (imm8 & 0x1) {
    return __a.v1;
  } else {
    return __a.v0;
  }
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm256_extractf128_pd(__m256d __a, const int imm8) {
  if (imm8 & 0x1) {
    return __a.v1;
  } else {
    return __a.v0;
  }
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm256_extractf128_si256(__m256i __a, const int imm8) {
  if (imm8 & 0x1) {
    return __a.v1;
  } else {
    return __a.v0;
  }
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_set_m128(__m128 __hi, __m128 __lo) {
  __m256 ret;
  ret.v0 = __lo;
  ret.v1 = __hi;
  return ret;
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_set_m128d(__m128d __hi, __m128d __lo) {
  __m256d ret;
  ret.v0 = __lo;
  ret.v1 = __hi;
  return ret;
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_set_m128i(__m128i __hi, __m128i __lo) {
  __m256i ret;
  ret.v0 = __lo;
  ret.v1 = __hi;
  return ret;
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_setr_m128(__m128 __lo, __m128 __hi) {
  return _mm256_set_m128(__hi, __lo);
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_setr_m128d(__m128d __lo, __m128d __hi) {
  return (__m256d)_mm256_set_m128d(__hi, __lo);
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_setr_m128i(__m128i __lo, __m128i __hi) {
  return (__m256i)_mm256_set_m128i(__hi, __lo);
}

static __inline__ __m256 __attribute__((__always_inline__, __nodebug__))
_mm256_loadu2_m128(float const* __addr_hi, float const* __addr_lo) {
  return _mm256_set_m128(_mm_loadu_ps(__addr_hi), _mm_loadu_ps(__addr_lo));
}

static __inline__ __m256d __attribute__((__always_inline__, __nodebug__))
_mm256_loadu2_m128d(double const* __addr_hi, double const* __addr_lo) {
  return _mm256_set_m128d(_mm_loadu_pd(__addr_hi), _mm_loadu_pd(__addr_lo));
}

static __inline__ __m256i __attribute__((__always_inline__, __nodebug__))
_mm256_loadu2_m128i(__m128i_u const* __addr_hi, __m128i_u const* __addr_lo) {
  return _mm256_set_m128i(_mm_loadu_si128((__m128i const*)__addr_hi),
                          _mm_loadu_si128((__m128i const*)__addr_lo));
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_storeu2_m128(float* __addr_hi, float* __addr_lo, __m256 __a) {
  _mm_storeu_ps(__addr_lo, __a.v0);
  _mm_storeu_ps(__addr_hi, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_storeu2_m128d(double* __addr_hi, double* __addr_lo, __m256d __a) {
  _mm_storeu_pd(__addr_lo, __a.v0);
  _mm_storeu_pd(__addr_hi, __a.v1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm256_storeu2_m128i(__m128i_u* __addr_hi, __m128i_u* __addr_lo, __m256i __a) {
  _mm_storeu_si128((__m128i*)__addr_lo, __a.v0);
  _mm_storeu_si128((__m128i*)__addr_hi, __a.v1);
}

#endif /* __emscripten_avxintrin_h__ */
