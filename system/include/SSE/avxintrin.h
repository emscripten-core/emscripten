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

#include <nmmintrin.h>

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_broadcast_ss(const float *__mem_addr)
{
  return (__m128)wasm_v32x4_load_splat(__mem_addr);
}

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
#define _CMP_NGE_UQ  25
#define _CMP_NGT_UQ  26
#define _CMP_FALSE_OS  27
#define _CMP_NEQ_OS  28
#define _CMP_GE_OQ 29
#define _CMP_GT_OQ 30
#define _CMP_TRUE_US 31

#define _mm_cmp_pd(__a, __b, __imm) __extension__ ({ \
     __m128d __ret; \
     if ((__imm) == _CMP_EQ_OQ || (__imm) == _CMP_EQ_OS) __ret = _mm_cmpeq_pd((__a), (__b)); \
     if ((__imm) == _CMP_EQ_UQ || (__imm) == _CMP_EQ_US) __ret = _mm_or_pd(_mm_cmpeq_pd((__a), (__b)), _mm_cmpunord_pd((__a), (__b))); \
     if ((__imm) == _CMP_LT_OS || (__imm) == _CMP_LT_OQ) __ret = _mm_cmplt_pd((__a), (__b)); \
     if ((__imm) == _CMP_LE_OS || (__imm) == _CMP_LE_OQ) __ret = _mm_cmple_pd((__a), (__b)); \
     if ((__imm) == _CMP_UNORD_Q || (__imm) == _CMP_UNORD_S) __ret = _mm_cmpunord_pd((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_UQ || (__imm) == _CMP_NEQ_US) __ret = _mm_cmpneq_pd((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_OQ || (__imm) == _CMP_NEQ_OS) __ret = _mm_andnot_pd(_mm_cmpunord_pd((__a), (__b)), _mm_cmpneq_pd((__a), (__b))); \
     if ((__imm) == _CMP_NLT_US || (__imm) == _CMP_NLT_UQ) __ret = _mm_cmpnlt_pd((__a), (__b)); \
     if ((__imm) == _CMP_ORD_Q || (__imm) == _CMP_ORD_S) __ret = _mm_cmpord_pd((__a), (__b)); \
     if ((__imm) == _CMP_NGE_US || (__imm) == _CMP_NGE_UQ) __ret = _mm_cmpnge_pd((__a), (__b)); \
     if ((__imm) == _CMP_NGT_US || (__imm) == _CMP_NGT_UQ) __ret = _mm_cmpngt_pd((__a), (__b)); \
     if ((__imm) == _CMP_FALSE_OQ || (__imm) == _CMP_FALSE_OS) __ret = _mm_setzero_pd(); \
     if ((__imm) == _CMP_GE_OS || (__imm) == _CMP_GE_OQ) __ret = _mm_cmpge_pd((__a), (__b)); \
     if ((__imm) == _CMP_GT_OS || (__imm) == _CMP_GT_OQ) __ret = _mm_cmpgt_pd((__a), (__b)); \
     if ((__imm) == _CMP_TRUE_UQ || (__imm) == _CMP_TRUE_US) __ret = (__m128d)wasm_i8x16_splat(0xFF); \
     if ((__imm) == _CMP_NLE_US || (__imm) == _CMP_NLE_UQ) __ret = _mm_cmpnle_pd((__a), (__b)); \
     __ret; })

#define _mm_cmp_ps(__a, __b, __imm) __extension__ ({ \
     __m128 __ret; \
     if ((__imm) == _CMP_EQ_OQ || (__imm) == _CMP_EQ_OS) __ret = _mm_cmpeq_ps((__a), (__b)); \
     if ((__imm) == _CMP_EQ_UQ || (__imm) == _CMP_EQ_US) __ret = _mm_or_ps(_mm_cmpeq_ps((__a), (__b)), _mm_cmpunord_ps((__a), (__b))); \
     if ((__imm) == _CMP_LT_OS || (__imm) == _CMP_LT_OQ) __ret = _mm_cmplt_ps((__a), (__b)); \
     if ((__imm) == _CMP_LE_OS || (__imm) == _CMP_LE_OQ) __ret = _mm_cmple_ps((__a), (__b)); \
     if ((__imm) == _CMP_UNORD_Q || (__imm) == _CMP_UNORD_S) __ret = _mm_cmpunord_ps((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_UQ || (__imm) == _CMP_NEQ_US) __ret = _mm_cmpneq_ps((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_OQ || (__imm) == _CMP_NEQ_OS) __ret = _mm_andnot_ps(_mm_cmpunord_ps((__a), (__b)), _mm_cmpneq_ps((__a), (__b))); \
     if ((__imm) == _CMP_NLT_US || (__imm) == _CMP_NLT_UQ) __ret = _mm_cmpnlt_ps((__a), (__b)); \
     if ((__imm) == _CMP_ORD_Q || (__imm) == _CMP_ORD_S) __ret = _mm_cmpord_ps((__a), (__b)); \
     if ((__imm) == _CMP_NGE_US || (__imm) == _CMP_NGE_UQ) __ret = _mm_cmpnge_ps((__a), (__b)); \
     if ((__imm) == _CMP_NGT_US || (__imm) == _CMP_NGT_UQ) __ret = _mm_cmpngt_ps((__a), (__b)); \
     if ((__imm) == _CMP_FALSE_OQ || (__imm) == _CMP_FALSE_OS) __ret = _mm_setzero_ps(); \
     if ((__imm) == _CMP_GE_OS || (__imm) == _CMP_GE_OQ) __ret = _mm_cmpge_ps((__a), (__b)); \
     if ((__imm) == _CMP_GT_OS || (__imm) == _CMP_GT_OQ) __ret = _mm_cmpgt_ps((__a), (__b)); \
     if ((__imm) == _CMP_TRUE_UQ || (__imm) == _CMP_TRUE_US) __ret = (__m128)wasm_i8x16_splat(0xFF); \
     if ((__imm) == _CMP_NLE_US || (__imm) == _CMP_NLE_UQ) __ret = _mm_cmpnle_ps((__a), (__b)); \
     __ret; })

#define _mm_cmp_sd(__a, __b, __imm) __extension__ ({ \
     __m128d __ret; \
     if ((__imm) == _CMP_EQ_OQ || (__imm) == _CMP_EQ_OS) __ret = _mm_cmpeq_sd((__a), (__b)); \
     if ((__imm) == _CMP_EQ_UQ || (__imm) == _CMP_EQ_US) __ret = _mm_move_sd((__a), _mm_or_pd(_mm_cmpeq_sd((__a), (__b)), _mm_cmpunord_sd((__a), (__b)))); \
     if ((__imm) == _CMP_LT_OS || (__imm) == _CMP_LT_OQ) __ret = _mm_cmplt_sd((__a), (__b)); \
     if ((__imm) == _CMP_LE_OS || (__imm) == _CMP_LE_OQ) __ret = _mm_cmple_sd((__a), (__b)); \
     if ((__imm) == _CMP_UNORD_Q || (__imm) == _CMP_UNORD_S) __ret = _mm_cmpunord_sd((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_UQ || (__imm) == _CMP_NEQ_US) __ret = _mm_cmpneq_sd((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_OQ || (__imm) == _CMP_NEQ_OS) __ret = _mm_move_sd((__a), _mm_andnot_pd(_mm_cmpunord_sd((__a), (__b)), _mm_cmpneq_sd((__a), (__b)))); \
     if ((__imm) == _CMP_NLT_US || (__imm) == _CMP_NLT_UQ) __ret = _mm_cmpnlt_sd((__a), (__b)); \
     if ((__imm) == _CMP_ORD_Q || (__imm) == _CMP_ORD_S) __ret = _mm_cmpord_sd((__a), (__b)); \
     if ((__imm) == _CMP_NGE_US || (__imm) == _CMP_NGE_UQ) __ret = _mm_cmpnge_sd((__a), (__b)); \
     if ((__imm) == _CMP_NGT_US || (__imm) == _CMP_NGT_UQ) __ret = _mm_cmpngt_sd((__a), (__b)); \
     if ((__imm) == _CMP_FALSE_OQ || (__imm) == _CMP_FALSE_OS) __ret = _mm_move_sd((__a), _mm_setzero_pd()); \
     if ((__imm) == _CMP_GE_OS || (__imm) == _CMP_GE_OQ) __ret = _mm_cmpge_sd((__a), (__b)); \
     if ((__imm) == _CMP_GT_OS || (__imm) == _CMP_GT_OQ) __ret = _mm_cmpgt_sd((__a), (__b)); \
     if ((__imm) == _CMP_TRUE_UQ || (__imm) == _CMP_TRUE_US) __ret = _mm_move_sd((__a), (__m128d)wasm_i8x16_splat(0xFF)); \
     if ((__imm) == _CMP_NLE_US || (__imm) == _CMP_NLE_UQ) __ret = _mm_cmpnle_sd((__a), (__b)); \
     __ret; })

#define _mm_cmp_ss(__a, __b, __imm) __extension__ ({ \
     __m128 __ret; \
     if ((__imm) == _CMP_EQ_OQ || (__imm) == _CMP_EQ_OS) __ret = _mm_cmpeq_ss((__a), (__b)); \
     if ((__imm) == _CMP_EQ_UQ || (__imm) == _CMP_EQ_US) __ret = _mm_move_ss((__a), _mm_or_ps(_mm_cmpeq_ss((__a), (__b)), _mm_cmpunord_ss((__a), (__b)))); \
     if ((__imm) == _CMP_LT_OS || (__imm) == _CMP_LT_OQ) __ret = _mm_cmplt_ss((__a), (__b)); \
     if ((__imm) == _CMP_LE_OS || (__imm) == _CMP_LE_OQ) __ret = _mm_cmple_ss((__a), (__b)); \
     if ((__imm) == _CMP_UNORD_Q || (__imm) == _CMP_UNORD_S) __ret = _mm_cmpunord_ss((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_UQ || (__imm) == _CMP_NEQ_US) __ret = _mm_cmpneq_ss((__a), (__b)); \
     if ((__imm) == _CMP_NEQ_OQ || (__imm) == _CMP_NEQ_OS) __ret = _mm_move_ss((__a), _mm_andnot_ps(_mm_cmpunord_ss((__a), (__b)), _mm_cmpneq_ss((__a), (__b)))); \
     if ((__imm) == _CMP_NLT_US || (__imm) == _CMP_NLT_UQ) __ret = _mm_cmpnlt_ss((__a), (__b)); \
     if ((__imm) == _CMP_ORD_Q || (__imm) == _CMP_ORD_S) __ret = _mm_cmpord_ss((__a), (__b)); \
     if ((__imm) == _CMP_NGE_US || (__imm) == _CMP_NGE_UQ) __ret = _mm_cmpnge_ss((__a), (__b)); \
     if ((__imm) == _CMP_NGT_US || (__imm) == _CMP_NGT_UQ) __ret = _mm_cmpngt_ss((__a), (__b)); \
     if ((__imm) == _CMP_FALSE_OQ || (__imm) == _CMP_FALSE_OS) __ret = _mm_move_ss((__a), _mm_setzero_ps()); \
     if ((__imm) == _CMP_GE_OS || (__imm) == _CMP_GE_OQ) __ret = _mm_cmpge_ss((__a), (__b)); \
     if ((__imm) == _CMP_GT_OS || (__imm) == _CMP_GT_OQ) __ret = _mm_cmpgt_ss((__a), (__b)); \
     if ((__imm) == _CMP_TRUE_UQ || (__imm) == _CMP_TRUE_US) __ret = _mm_move_ss((__a), (__m128)wasm_i8x16_splat(0xFF)); \
     if ((__imm) == _CMP_NLE_US || (__imm) == _CMP_NLE_UQ) __ret = _mm_cmpnle_ss((__a), (__b)); \
     __ret; })

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_maskload_pd(const double *__mem_addr, __m128i __mask)
{
  // This may cause an out-of-bounds memory load since we first load and
  // then mask, but since there are no segmentation faults in Wasm memory
  // accesses, that is ok (as long as we are within the heap bounds -
  // a negligible limitation in practice)
  return _mm_and_pd(_mm_load_pd(__mem_addr), (__m128d)wasm_i64x2_shr(__mask, 63));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_maskload_ps(const float *__mem_addr, __m128i __mask)
{
  // This may cause an out-of-bounds memory load since we first load and
  // then mask, but since there are no segmentation faults in Wasm memory
  // accesses, that is ok (as long as we are within the heap bounds -
  // a negligible limitation in practice)
  return _mm_and_ps(_mm_load_ps(__mem_addr), (__m128)_mm_srai_epi32(__mask, 31));
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_maskstore_pd(double *__mem_addr, __m128i __mask, __m128d __a)
{
  if ((wasm_i64x2_extract_lane(__mask, 0) & 0x8000000000000000ull) != 0) __mem_addr[0] = wasm_f64x2_extract_lane(__a, 0);
  if ((wasm_i64x2_extract_lane(__mask, 1) & 0x8000000000000000ull) != 0) __mem_addr[1] = wasm_f64x2_extract_lane(__a, 1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_maskstore_ps(float *__mem_addr, __m128i __mask, __m128 __a)
{
  if ((wasm_i32x4_extract_lane(__mask, 0) & 0x80000000ull) != 0) __mem_addr[0] = wasm_f32x4_extract_lane(__a, 0);
  if ((wasm_i32x4_extract_lane(__mask, 1) & 0x80000000ull) != 0) __mem_addr[1] = wasm_f32x4_extract_lane(__a, 1);
  if ((wasm_i32x4_extract_lane(__mask, 2) & 0x80000000ull) != 0) __mem_addr[2] = wasm_f32x4_extract_lane(__a, 2);
  if ((wasm_i32x4_extract_lane(__mask, 3) & 0x80000000ull) != 0) __mem_addr[3] = wasm_f32x4_extract_lane(__a, 3);
}

#define _mm_permute_pd(__a, __imm) __extension__ ({ \
  (__m128d)wasm_v64x2_shuffle((__m128d)(__a), (__m128d)(__a), \
                              ((__imm) & 1), (((__imm) >> 1) & 1)); })

#define _mm_permute_ps(__a, __imm) __extension__ ({ \
  (__m128)wasm_v32x4_shuffle((__m128)(__a), (__m128)(__a), \
                             ((__imm) & 3), (((__imm) >> 2) & 3), \
                             (((__imm) >> 4) & 3), (((__imm) >> 6) & 3)); })

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_permutevar_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_make(
      ((__f64x2)__a)[(wasm_i64x2_extract_lane(__b, 0) >> 1) & 1],
      ((__f64x2)__a)[(wasm_i64x2_extract_lane(__b, 1) >> 1) & 1]);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_permutevar_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_make(
      ((__f32x4)__a)[wasm_i32x4_extract_lane(__b, 0) & 3],
      ((__f32x4)__a)[wasm_i32x4_extract_lane(__b, 1) & 3],
      ((__f32x4)__a)[wasm_i32x4_extract_lane(__b, 2) & 3],
      ((__f32x4)__a)[wasm_i32x4_extract_lane(__b, 3) & 3]);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testc_pd(__m128d __a, __m128d __b)
{
  v128_t __m = wasm_u64x2_shr(wasm_v128_or(wasm_v128_not((v128_t)__b), (v128_t)__a), 63);
  return wasm_i64x2_extract_lane(__m, 0) & wasm_i64x2_extract_lane(__m, 1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testc_ps(__m128 __a, __m128 __b)
{
  v128_t __m = wasm_u32x4_shr(wasm_v128_or(wasm_v128_not((v128_t)__b), (v128_t)__a), 31);
  __m = wasm_v128_and(__m, (v128_t)_mm_movehl_ps((__m128)__m, (__m128)__m));
  __m = wasm_v128_and(__m, _mm_shuffle_epi32(__m, _MM_SHUFFLE(3, 2, 0, 1)));
  return wasm_i32x4_extract_lane(__m, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testnzc_pd(__m128d __a, __m128d __b)
{
  v128_t  __m = wasm_u64x2_shr(wasm_v128_and((v128_t)__a, (v128_t)__b), 63);
  v128_t __m2 = wasm_u64x2_shr(wasm_v128_andnot((v128_t)__b, (v128_t)__a), 63);
  return (wasm_i64x2_extract_lane(__m, 0)  | wasm_i64x2_extract_lane(__m, 1))
       & (wasm_i64x2_extract_lane(__m2, 0) | wasm_i64x2_extract_lane(__m2, 1));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testnzc_ps(__m128 __a, __m128 __b)
{
  v128_t  __m = wasm_u32x4_shr(wasm_v128_and((v128_t)__a, (v128_t)__b), 31);
  v128_t __m2 = wasm_u32x4_shr(wasm_v128_andnot((v128_t)__b, (v128_t)__a), 31);

  __m  = wasm_v128_or(__m,  (v128_t)_mm_movehl_ps((__m128)__m, (__m128)__m));
  __m2 = wasm_v128_or(__m2, (v128_t)_mm_movehl_ps((__m128)__m2, (__m128)__m2));
  __m  = wasm_v128_or(__m,  _mm_shuffle_epi32(__m, _MM_SHUFFLE(3, 2, 0, 1)));
  __m2 = wasm_v128_or(__m2, _mm_shuffle_epi32(__m2, _MM_SHUFFLE(3, 2, 0, 1)));

  return wasm_i32x4_extract_lane(__m, 0) & wasm_i32x4_extract_lane(__m2, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testz_pd(__m128d __a, __m128d __b)
{
  v128_t __m = wasm_u64x2_shr(wasm_v128_not(wasm_v128_and((v128_t)__a, (v128_t)__b)), 63);
  return wasm_i64x2_extract_lane(__m, 0) & wasm_i64x2_extract_lane(__m, 1);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_testz_ps(__m128 __a, __m128 __b)
{
  v128_t __m = wasm_u32x4_shr(wasm_v128_not(wasm_v128_and((v128_t)__a, (v128_t)__b)), 31);
  __m = wasm_v128_and(__m, (v128_t)_mm_movehl_ps((__m128)__m, (__m128)__m));
  __m = wasm_v128_and(__m, _mm_shuffle_epi32(__m, _MM_SHUFFLE(3, 2, 0, 1)));
  return wasm_i32x4_extract_lane(__m, 0);
}

#endif /* __emscripten_avxintrin_h__ */
