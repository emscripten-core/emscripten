/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_emmintrin_h__
#define __emscripten_emmintrin_h__

#ifndef __SSE2__
#error "SSE2 instruction set not enabled"
#endif

#include <xmmintrin.h>

// Alias different (functionally) equivalent intrinsics.
#define _mm_set_epi64x _mm_set_epi64
#define _mm_cvtsd_si64x _mm_cvtsd_si64
#define _mm_cvtsi128_si64x _mm_cvtsi128_si64
#define _mm_cvtsi64x_sd _mm_cvtsi64_sd
#define _mm_cvtsi64x_si128 _mm_cvtsi64_si128
#define _mm_cvttsd_si64x _mm_cvttsd_si64
#define _mm_store_pd1 _mm_store1_pd

typedef __f64x2 __m128d;

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_move_sd(__m128d __a, __m128d __b)
{
  return (__m128d){ __b[0], __a[1] };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_add_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_add((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_add_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_add_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sub_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_sub((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sub_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_sub_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_mul_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_mul((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_mul_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_mul_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_div_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_div((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_div_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_div_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_pd(__m128d __a)
{
  return (__m128d)wasm_f64x2_sqrt((v128_t)__a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_sqrt_pd(__b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_min_pd(__m128d __a, __m128d __b)
{
//  return (__m128d)wasm_f32x4_pmin((v128_t)__a, (v128_t)__b); // TODO: Migrate to this, once it works in VMs
  return (__m128d)wasm_v128_bitselect((v128_t)__a, (v128_t)__b, (v128_t)wasm_f64x2_lt((v128_t)__a, (v128_t)__b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_min_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_min_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_max_pd(__m128d __a, __m128d __b)
{
//  return (__m128)wasm_f32x4_pmax((v128_t)__a, (v128_t)__b); // TODO: Migrate to this, once it works in VMs
  return (__m128d)wasm_v128_bitselect((v128_t)__a, (v128_t)__b, (v128_t)wasm_f64x2_gt((v128_t)__a, (v128_t)__b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_max_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_max_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_and_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_and((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_andnot_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_andnot((v128_t)__b, (v128_t)__a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_or_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_or((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_xor_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_xor((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_eq((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_lt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmple_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_le((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_gt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_ge((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_and(wasm_f64x2_eq((v128_t)__a, (v128_t)__a),
                                wasm_f64x2_eq((v128_t)__b, (v128_t)__b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_or(wasm_f64x2_ne((v128_t)__a, (v128_t)__a),
                               wasm_f64x2_ne((v128_t)__b, (v128_t)__b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_f64x2_ne((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_not((v128_t)_mm_cmplt_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_not((v128_t)_mm_cmple_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_not((v128_t)_mm_cmpgt_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_v128_not((v128_t)_mm_cmpge_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpeq_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmplt_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmple_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmple_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpgt_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpge_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpord_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpunord_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpneq_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpnlt_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpnle_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpngt_pd(__a, __b));
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_sd(__m128d __a, __m128d __b)
{
  return _mm_move_sd(__a, _mm_cmpnge_pd(__a, __b));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comieq_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) == wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comilt_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) < wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comile_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) <= wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comigt_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) > wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comige_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) >= wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comineq_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) != wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomieq_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) == wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomilt_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) < wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomile_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) <= wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomigt_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) > wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomige_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) >= wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomineq_sd(__m128d __a, __m128d __b)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0) != wasm_f64x2_extract_lane((v128_t)__b, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_ps(__m128d __a)
{
  return (__m128)wasm_f32x4_demote_f64x2_zero((v128_t)__a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_pd(__m128 __a)
{
  return (__m128d)wasm_f64x2_promote_low_f32x4((v128_t)__a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_pd(__m128i __a)
{
  return (__m128d)wasm_f64x2_convert_low_i32x4((v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_epi32(__m128d __a)
{
  // TODO: OPTIMIZE!
  int m[2];
  for(int i = 0; i < 2; ++i)
  {
    int x = lrint(__a[i]);
    if (x != 0 || fabs(__a[i]) < 2.0)
      m[i] = (int)x;
    else
      m[i] = (int)0x80000000;
  }
  return wasm_i32x4_make(m[0], m[1], 0, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si32(__m128d __a)
{
  // TODO: OPTIMIZE!
  int x = lrint(__a[0]);
  if (x != 0 || fabs(__a[0]) < 2.0)
    return (int)x;
  else
    return (int)0x80000000;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_ss(__m128 __a, __m128d __b)
{
  __a[0] = __b[0];
  return __a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_sd(__m128d __a, int __b)
{
  __a[0] = __b;
  return __a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_sd(__m128d __a, __m128 __b)
{
  __a[0] = __b[0];
  return __a;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvttpd_epi32(__m128d __a)
{
  // TODO: OPTIMIZE!
  int m[2];
  for(int i = 0; i < 2; ++i)
  {
    int x = lrint(__a[i]);
    if (x != 0 || fabs(__a[i]) < 2.0)
      m[i] = (int)__a[i];
    else
      m[i] = (int)0x80000000;
  }
  return wasm_i32x4_make(m[0], m[1], 0, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si32(__m128d __a)
{
  // TODO: OPTIMIZE!
  int x = lrint(__a[0]);
  if (x != 0 || fabs(__a[0]) < 2.0)
    return (int)__a[0];
  else
    return (int)0x80000000;
}

static __inline__ double __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_f64(__m128d __a)
{
  return wasm_f64x2_extract_lane((v128_t)__a, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load_pd(double const *__dp)
{
  return *(__m128d*)__dp;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load1_pd(double const *__dp)
{
  return (__m128d)wasm_v64x2_load_splat(__dp);
}

#define        _mm_load_pd1(dp)        _mm_load1_pd(dp)

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadr_pd(double const *__p)
{
  __m128d __u = *(__m128d*)__p; // aligned load
  return (__m128d)wasm_i64x2_shuffle((v128_t)__u, (v128_t)__u, 1, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadu_pd(double const *__dp)
{
  struct __loadu_pd {
    __m128d __v;
  } __attribute__((__packed__, __may_alias__));
  return ((struct __loadu_pd*)__dp)->__v;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load_sd(double const *__p)
{
  return (__m128d)wasm_v128_load64_zero((const void*)__p);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadh_pd(__m128d __a, double const *__dp)
{
  return (__m128d)wasm_v128_load64_lane((const void*)__dp, (v128_t)__a, 1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadl_pd(__m128d __a, double const *__dp)
{
  return (__m128d)wasm_v128_load64_lane((const void*)__dp, (v128_t)__a, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set_sd(double __w)
{
  return (__m128d)wasm_f64x2_make(__w, 0);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set1_pd(double __w)
{
  return (__m128d)wasm_f64x2_splat(__w);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set_pd(double __c1, double __c0)
{
  return (__m128d)wasm_f64x2_make(__c0, __c1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_setr_pd(double __c0, double __c1)
{
  return (__m128d)wasm_f64x2_make(__c0, __c1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_setzero_pd(void)
{
  return (__m128d)wasm_f64x2_const(0.0, 0.0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_sd(double *__dp, __m128d __a)
{
  wasm_v128_store64_lane((void*)__dp, (v128_t)__a, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store1_pd(double *__dp, __m128d __a)
{
  struct __mm_store1_pd_struct {
    double __u[2];
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_store1_pd_struct*)__dp)->__u[0] = __a[0];
  ((struct __mm_store1_pd_struct*)__dp)->__u[1] = __a[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_pd(double *__dp, __m128d __a)
{
  *(__m128d *)__dp = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_pd(double *__dp, __m128d __a)
{
  struct __unaligned {
    __m128d __v;
  } __attribute__((__packed__, __may_alias__));

  ((struct __unaligned *)__dp)->__v = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storer_pd(double *__p, __m128d __a)
{
  *(__m128d *)__p = (__m128d)wasm_i64x2_shuffle((v128_t)__a, (v128_t)__a, 1, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeh_pd(double *__dp, __m128d __a)
{
  wasm_v128_store64_lane((void*)__dp, (v128_t)__a, 1);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_pd(double *__dp, __m128d __a)
{
  wasm_v128_store64_lane((void*)__dp, (v128_t)__a, 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_add((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_add((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_add((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi64(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i64x2_add((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_add_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_add_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epu8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u8x16_add_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epu16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u16x8_add_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_avg_epu8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u8x16_avgr((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_avg_epu16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u16x8_avgr((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_madd_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_dot_i16x8((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_max((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u8x16_max((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_min((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u8x16_min((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_epi16(__m128i __a, __m128i __b)
{
  const v128_t lo = wasm_i32x4_extmul_low_i16x8((v128_t)__a, (v128_t)__b);
  const v128_t hi = wasm_i32x4_extmul_high_i16x8((v128_t)__a, (v128_t)__b);
  return (__m128i)wasm_i16x8_shuffle(lo, hi, 1, 3, 5, 7, 9, 11, 13, 15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_epu16(__m128i __a, __m128i __b)
{
  const v128_t lo = wasm_u32x4_extmul_low_u16x8((v128_t)__a, (v128_t)__b);
  const v128_t hi = wasm_u32x4_extmul_high_u16x8((v128_t)__a, (v128_t)__b);
  return (__m128i)wasm_i16x8_shuffle(lo, hi, 1, 3, 5, 7, 9, 11, 13, 15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mullo_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_mul((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epu32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u64x2_extmul_low_u32x4(
      wasm_v32x4_shuffle((v128_t)__a, (v128_t)__a, 0, 2, 0, 2),
      wasm_v32x4_shuffle((v128_t)__b, (v128_t)__b, 0, 2, 0, 2));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_sub((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_sub((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_sub((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi64(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i64x2_sub((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_sub_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_sub_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epu8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u8x16_sub_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epu16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_u16x8_sub_saturate((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_and_si128(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_v128_and((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_andnot_si128(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_v128_andnot((v128_t)__b, (v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_or_si128(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_v128_or((v128_t)__b, (v128_t)__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_xor_si128(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_v128_xor((v128_t)__b, (v128_t)__a);
}

#define _mm_slli_si128(__a, __imm) __extension__ ({               \
  (__m128i)wasm_i8x16_shuffle(_mm_setzero_si128(),                \
                             (__a),                               \
                             ((__imm)&0xF0) ? 0 : 16 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 17 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 18 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 19 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 20 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 21 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 22 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 23 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 24 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 25 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 26 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 27 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 28 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 29 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 30 - ((__imm)&0xF), \
                             ((__imm)&0xF0) ? 0 : 31 - ((__imm)&0xF)); })
#define _mm_bslli_si128(__a, __imm) \
  _mm_slli_si128((__a), (__imm))

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi16(__m128i __a, int __count)
{
  return (__m128i)((__count < 16) ? wasm_i16x8_shl((v128_t)__a, __count) : wasm_i16x8_const(0,0,0,0,0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi16(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  return (__m128i)((__c < 16) ? wasm_i16x8_shl((v128_t)__a, __c) : wasm_i16x8_const(0,0,0,0,0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi32(__m128i __a, int __count)
{
  return (__m128i)((__count < 32) ? wasm_i32x4_shl((v128_t)__a, __count) : wasm_i32x4_const(0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi32(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  return (__m128i)((__c < 32) ? wasm_i32x4_shl((v128_t)__a, __c) : wasm_i32x4_const(0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi64(__m128i __a, int __count)
{
  return (__m128i)((__count < 64) ? wasm_i64x2_shl((v128_t)__a, __count) : wasm_i64x2_const(0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi64(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  return (__m128i)((__c < 64) ? wasm_i64x2_shl((v128_t)__a, __c) : wasm_i64x2_const(0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srai_epi16(__m128i __a, int __count)
{
  __count = __count < 15 ? __count : 15;
  return (__m128i)wasm_i16x8_shr((v128_t)__a, __count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sra_epi16(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  __c = __c < 15 ? __c : 15;
  return (__m128i)wasm_i16x8_shr((v128_t)__a, __c);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srai_epi32(__m128i __a, int __count)
{
  __count = __count < 31 ? __count : 31;
  return (__m128i)wasm_i32x4_shr((v128_t)__a, __count);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sra_epi32(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  __c = __c < 31 ? __c : 31;
  return (__m128i)wasm_i32x4_shr((v128_t)__a, __c);
}

#define _mm_srli_si128(__a, __imm) __extension__ ({                     \
  (__m128i)wasm_i8x16_shuffle((__a),                                    \
                              _mm_setzero_si128(),                      \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 0,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 1,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 2,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 3,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 4,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 5,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 6,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 7,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 8,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 9,  \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 10, \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 11, \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 12, \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 13, \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 14, \
                              ((__imm)&0xF0) ? 16 : ((__imm)&0xF) + 15); })

#define _mm_bsrli_si128(__a, __imm) \
  _mm_srli_si128((__a), (__imm))

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi16(__m128i __a, int __count)
{
  return (__m128i)(((unsigned int)__count < 16) ? wasm_u16x8_shr((v128_t)__a, __count) : wasm_i16x8_const(0,0,0,0,0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi16(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  return (__m128i)((__c < 16) ? wasm_u16x8_shr((v128_t)__a, __c) : wasm_i16x8_const(0,0,0,0,0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi32(__m128i __a, int __count)
{
  return (__m128i)(((unsigned int)__count < 32) ? wasm_u32x4_shr((v128_t)__a, __count) : wasm_i32x4_const(0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi32(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  return (__m128i)((__c < 32) ? wasm_u32x4_shr((v128_t)__a, __c) : wasm_i32x4_const(0,0,0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi64(__m128i __a, int __count)
{
  return (__m128i)(((unsigned int)__count < 64) ? wasm_u64x2_shr((v128_t)__a, __count) : wasm_i64x2_const(0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi64(__m128i __a, __m128i __count)
{
  unsigned long long __c = (unsigned long long)((__u64x2)__count)[0];
  return (__m128i)((__c < 64) ? wasm_u64x2_shr((v128_t)__a, __c) : wasm_i64x2_const(0,0));
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_eq((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_eq((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_eq((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_gt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_gt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_gt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_lt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_lt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_lt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_sd(__m128d __a, long long __b)
{
  // TODO: optimize
  union {
    double x[2];
    __m128d m;
  } m;
  m.m = __a;
  m.x[0] = (double)__b;
  return m.m;
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si64(__m128d __a)
{
  // TODO: optimize
  if (isnan(__a[0]) || isinf(__a[0])) return 0x8000000000000000LL;
  long long x = llrint(__a[0]);
  if (x != 0xFFFFFFFF00000000ULL && (x != 0 || fabsf(__a[0]) < 2.f))
    return x;
  else
    return 0x8000000000000000LL;
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si64(__m128d __a)
{
  // TODO: optimize
  if (isnan(__a[0]) || isinf(__a[0])) return 0x8000000000000000LL;
  long long x = llrint(__a[0]);
  if (x != 0xFFFFFFFF00000000ULL && (x != 0 || fabsf(__a[0]) < 2.f))
    return (long long)__a[0];
  else
    return 0x8000000000000000LL;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_ps(__m128i __a)
{
  return (__m128)wasm_f32x4_convert_i32x4(__a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_epi32(__m128 __a)
{
  // TODO: optimize
  union {
    int x[4];
    __m128i m;
  } u;
  for(int i = 0; i < 4; ++i)
  {
    int x = lrint(__a[i]);
    if (x != 0 || fabs(__a[i]) < 2.0)
      u.x[i] = x;
    else
      u.x[i] = (int)0x80000000;
  }
  return u.m;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvttps_epi32(__m128 __a)
{
  // TODO: optimize
  union {
    int x[4];
    __m128i m;
  } u;
  for(int i = 0; i < 4; ++i)
  {
    int x = lrint(__a[i]);
    if (x != 0 || fabs(__a[i]) < 2.0)
      u.x[i] = (int)__a[i];
    else
      u.x[i] = (int)0x80000000;
  }
  return u.m;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_si128(int __a)
{
  return (__m128i)wasm_i32x4_make(__a, 0, 0, 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_si128(long long __a)
{
  return (__m128i)wasm_i64x2_make(__a, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si32(__m128i __a)
{
  return wasm_i32x4_extract_lane(__a, 0);
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si64(__m128i __a)
{
  return wasm_i64x2_extract_lane(__a, 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_load_si128(__m128i const *__p)
{
  return *__p;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadu_si128(__m128i const *__p)
{
  // UB-free unaligned access copied from wasm_simd128.h
  struct __mm_loadu_si128_struct {
    __m128i __v;
  } __attribute__((__packed__, __may_alias__));
  return ((struct __mm_loadu_si128_struct*)__p)->__v;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadu_si16(void const *__p)
{
  return (__m128i)wasm_v128_load16_lane(__p, wasm_i64x2_const(0, 0), 0);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadu_si32(void const *__p)
{
  return (__m128i)wasm_v128_load32_zero(__p);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadu_si64(void const *__p)
{
  return (__m128i)wasm_v128_load64_zero(__p);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadl_epi64(__m128i const *__p)
{
  return _mm_loadu_si64(__p);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi64(long long q1, long long q0)
{
  return (__m128i)wasm_i64x2_make(q0, q1);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi32(int i3, int i2, int i1, int i0)
{
  return (__m128i)wasm_i32x4_make(i0, i1, i2, i3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi16(short w7, short w6, short w5, short w4, short w3, short w2, short w1, short w0)
{
  return (__m128i)wasm_i16x8_make(w0, w1, w2, w3, w4, w5, w6, w7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi8(char b15, char b14, char b13, char b12, char b11, char b10, char b9, char b8, char b7, char b6, char b5, char b4, char b3, char b2, char b1, char b0)
{
  return (__m128i)wasm_i8x16_make(b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi64x(long long __q)
{
  return (__m128i)wasm_i64x2_splat(__q);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi32(int __i)
{
  return (__m128i)wasm_i32x4_splat(__i);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi16(short __w)
{
  return (__m128i)wasm_i16x8_splat(__w);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi8(char __b)
{
  return (__m128i)wasm_i8x16_splat(__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi32(int i0, int i1, int i2, int i3)
{
  return (__m128i)wasm_i32x4_make(i0, i1, i2, i3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi16(short w0, short w1, short w2, short w3, short w4, short w5, short w6, short w7)
{
  return (__m128i)wasm_i16x8_make(w0, w1, w2, w3, w4, w5, w6, w7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi8(char b0, char b1, char b2, char b3, char b4, char b5, char b6, char b7, char b8, char b9, char b10, char b11, char b12, char b13, char b14, char b15)
{
  return (__m128i)wasm_i8x16_make(b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setzero_si128(void)
{
  return wasm_i64x2_const(0, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_si128(__m128i *__p, __m128i __b)
{
  *__p = __b;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si16(void *__p, __m128i __a)
{
  wasm_v128_store16_lane(__p, (v128_t)__a, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si32(void *__p, __m128i __a)
{
  wasm_v128_store32_lane(__p, (v128_t)__a, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si64(void *__p, __m128i __a)
{
  wasm_v128_store64_lane(__p, (v128_t)__a, 0);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si128(__m128i *__p, __m128i __a)
{
  // UB-free unaligned access copied from wasm_simd128.h
  struct __mm_storeu_si128_struct {
    __m128i __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_storeu_si128_struct *)__p)->__v = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_maskmoveu_si128(__m128i __d, __m128i __n, char *__p)
{
  // TODO: optimize
  union {
    unsigned char x[16];
    __m128i m;
  } mask, data;
  mask.m = __n;
  data.m = __d;
  for(int i = 0; i < 16; ++i)
    if (mask.x[i] & 0x80)
      __p[i] = data.x[i];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_epi64(__m128i *__p, __m128i __a)
{
  _mm_storeu_si64(__p, __a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_pd(double *__p, __m128d __a)
{
  // Emscripten/SIMD.js does not have cache hinting.
  _mm_store_pd(__p, __a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si128(__m128i *__p, __m128i __a)
{
  // Emscripten/SIMD.js does not have cache hinting.
  _mm_store_si128(__p, __a);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si32(int *__p, int __a)
{
  // No cache hinting available.
  *__p = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si64(long long *__p, long long __a)
{
  // No cache hinting available.
  *__p = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_clflush(void const *__p)
{
  // Wasm SIMD does not have cache hinting
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_lfence(void)
{
  __sync_synchronize(); // Wasm/SharedArrayBuffer has only a full barrier instruction, which gives a stronger guarantee.
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_mfence(void)
{
  __sync_synchronize(); // Wasm/SharedArrayBuffer has only a full barrier instruction, which gives a stronger guarantee.
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packs_epi16(__m128i __a, __m128i __b)
{
  return wasm_i8x16_narrow_i16x8(__a, __b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packs_epi32(__m128i __a, __m128i __b)
{
  return wasm_i16x8_narrow_i32x4(__a, __b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packus_epi16(__m128i __a, __m128i __b)
{
  return wasm_u8x16_narrow_i16x8(__a, __b);
}

#define _mm_extract_epi16(__a, __imm) wasm_u16x8_extract_lane((v128_t)(__a), (__imm) & 7)
#define _mm_insert_epi16(__a, __b, __imm) wasm_i16x8_replace_lane((__a), (__imm) & 7, (__b))

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_epi8(__m128i __a)
{
  return (int)wasm_i8x16_bitmask((v128_t)__a);
}

#define _mm_shuffle_epi32(__a, __imm) __extension__ ({ \
  (__m128i)wasm_i32x4_shuffle((__a), \
                              _mm_set1_epi32(0), \
                              ((__imm) & 0x3), (((__imm) & 0xc) >> 2), \
                              (((__imm) & 0x30) >> 4), (((__imm) & 0xc0) >> 6)); })

#define _mm_shufflelo_epi16(__a, __imm) __extension__ ({ \
  (__m128i)wasm_i16x8_shuffle((__a), \
                              _mm_set1_epi16(0), \
                              ((__imm) & 0x3), (((__imm) & 0xc) >> 2), \
                              (((__imm) & 0x30) >> 4), (((__imm) & 0xc0) >> 6), \
                              4, 5, 6, 7); })

#define _mm_shufflehi_epi16(__a, __imm) __extension__ ({ \
  (__m128i)wasm_i16x8_shuffle((__a), \
                              _mm_set1_epi16(0), \
                              0, 1, 2, 3, \
                              (4 + (((__imm) & 0x03) >> 0)), \
                              (4 + (((__imm) & 0x0c) >> 2)), \
                              (4 + (((__imm) & 0x30) >> 4)), \
                              (4 + (((__imm) & 0xc0) >> 6))); })

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_shuffle(__a, __b, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_shuffle(__a, __b, 4, 12, 5, 13, 6, 14, 7, 15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_shuffle(__a, __b, 2, 6, 3, 7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi64(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i64x2_shuffle(__a, __b, 1, 3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i8x16_shuffle(__a, __b, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i16x8_shuffle(__a, __b, 0, 8, 1, 9, 2, 10, 3, 11);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i32x4_shuffle(__a, __b, 0, 4, 1, 5);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi64(__m128i __a, __m128i __b)
{
  return (__m128i)wasm_i64x2_shuffle(__a, __b, 0, 2);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_move_epi64(__m128i __a)
{
  return wasm_i64x2_shuffle(__a, wasm_i64x2_const(0, 0), 0, 2);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_i64x2_shuffle((v128_t)__a, (v128_t)__b, 1, 3);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_pd(__m128d __a, __m128d __b)
{
  return (__m128d)wasm_i64x2_shuffle((v128_t)__a, (v128_t)__b, 0, 2);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_pd(__m128d __a)
{
  return (int)wasm_i64x2_bitmask((v128_t)__a);
}

#define _mm_shuffle_pd(__a, __b, __i) __extension__ ({ \
  (__m128d) __builtin_shufflevector((__u64x2)(__a), (__u64x2)(__b), \
                                    (__i) & 1, \
                                    (((__i) & 2) >> 1) + 2); })

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_castpd_ps(__m128d __a)
{
  return (__m128)__a;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_castpd_si128(__m128d __a)
{
  return (__m128i)__a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_castps_pd(__m128 __a)
{
  return (__m128d)__a;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_castps_si128(__m128 __a)
{
  return (__m128i)__a;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_castsi128_ps(__m128i __a)
{
  return (__m128)__a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_castsi128_pd(__m128i __a)
{
  return (__m128d)__a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_pause(void)
{
  // No pause/wait instruction in Wasm/SIMD.
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_undefined_pd()
{
  __m128d val;
  return val;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_undefined_si128()
{
  __m128i val;
  return val;
}

// Must be in the very end as it uses other SSE2 intrinsics
static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sad_epu8(__m128i __a, __m128i __b)
{
  __m128i __diff = _mm_or_si128(_mm_subs_epu8(__a, __b),
                                _mm_subs_epu8(__b, __a));
  __diff = _mm_add_epi16(_mm_srli_epi16(__diff, 8),
                         _mm_and_si128(__diff, _mm_set1_epi16(0x00FF)));
  __diff = _mm_add_epi16(__diff, _mm_slli_epi32(__diff, 16));
  __diff = _mm_add_epi16(__diff, _mm_slli_epi64(__diff, 32));
  return _mm_srli_epi64(__diff, 48);
}

#define _MM_SHUFFLE2(x, y) (((x) << 1) | (y))

#endif /* __emscripten_emmintrin_h__ */
