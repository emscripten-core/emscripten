/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_xmmintrin_h__
#define __emscripten_xmmintrin_h__

#include <wasm_simd128.h>

#include <math.h>
#include <string.h>

#ifndef __SSE__
#error "SSE instruction set not enabled"
#endif

// Emscripten SIMD support doesn't support MMX/float32x2/__m64.
// However, we support loading and storing 2-vectors, so
// recognize the type at least.
typedef float __m64 __attribute__((__vector_size__(8), __aligned__(8)));
typedef __f32x4 __m128;
typedef __i32x4 __m128i;

#define __f32x4_shuffle(__a, __b, __c0, __c1, __c2, __c3)                   \
  ((v128_t)(__builtin_shufflevector((__f32x4)(__a), (__f32x4)(__b), __c0,   \
                                    __c1, __c2, __c3)))

// This is defined as a macro because __builtin_shufflevector requires its
// mask argument to be a compile-time constant.
#define _mm_shuffle_ps(__a, __b, __mask) __extension__ ({ \
  ((__m128)__f32x4_shuffle(__a, __b, \
                           (((__mask) >> 0) & 0x3) + 0, \
                           (((__mask) >> 2) & 0x3) + 0, \
                           (((__mask) >> 4) & 0x3) + 4, \
                           (((__mask) >> 6) & 0x3) + 4)); })

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ps(float __z, float __y, float __x, float __w)
{
  return (__m128)wasm_f32x4_make(__w, __x, __y, __z);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_setr_ps(float __z, float __y, float __x, float __w)
{
  return (__m128)wasm_f32x4_make(__z, __y, __x, __w);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ss(float __w)
{
  return (__m128)wasm_f32x4_make(__w, 0, 0, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ps1(float __w)
{
  return (__m128)wasm_f32x4_splat(__w);
}

#define _mm_set1_ps _mm_set_ps1

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_setzero_ps(void)
{
  return (__m128)wasm_f32x4_const(0.f, 0.f, 0.f, 0.f);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ps(const float *__p)
{
  return *(__m128*)__p;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadl_pi(__m128 __a, const void /*__m64*/ *__p)
{
  return (__m128)__f32x4_shuffle(wasm_f32x4_make(((float*)__p)[0], ((float*)__p)[1], 0.f, 0.f), __a, 0, 1, 6, 7);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadh_pi(__m128 __a, const void /*__m64*/ *__p)
{
  return (__m128)__f32x4_shuffle(__a, wasm_f32x4_make(((float*)__p)[0], ((float*)__p)[1], 0.f, 0.f), 0, 1, 4, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadr_ps(const float *__p)
{
  __m128 __v = _mm_load_ps(__p);
  return (__m128)__f32x4_shuffle(__v, __v, 3, 2, 1, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadu_ps(const float *__p)
{
  return (__m128)wasm_v128_load(__p);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ps1(const float *__p)
{
  return (__m128)wasm_v32x4_load_splat(__p);
}
#define _mm_load1_ps _mm_load_ps1

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ss(const float *__p)
{
  return (__m128)wasm_f32x4_make(*__p, 0.f, 0.f, 0.f);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_pi(__m64 *__p, __m128 __a)
{
  *__p = (__m64) { __a[0], __a[1] };
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movehl_ps(__m128 __a, __m128 __b);

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeh_pi(__m64 *__p, __m128 __a)
{
  _mm_storel_pi(__p, _mm_movehl_ps(__a, __a));
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ps(float *__p, __m128 __a)
{
  *(__m128 *)__p = __a;
}
// No NTA cache hint available.
#define _mm_stream_ps _mm_store_ps

#define _MM_HINT_T0 3
#define _MM_HINT_T1 2
#define _MM_HINT_T2 1
#define _MM_HINT_NTA 0
// No prefetch available, dummy it out.
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_prefetch(const void *__p, int __i)
{
  ((void)__p);
  ((void)__i);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_sfence(void)
{
  // Wasm/SharedArrayBuffer memory model is sequentially consistent.
  // Perhaps a future version of the spec can provide a related fence.
  __sync_synchronize();
}

#define _MM_SHUFFLE(w, z, y, x) (((w) << 6) | ((z) << 4) | ((y) << 2) | (x))

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storer_ps(float *__p, __m128 __a)
{
  _mm_store_ps(__p, _mm_shuffle_ps(__a, __a, _MM_SHUFFLE(0, 1, 2, 3)));
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ps1(float *__p, __m128 __a)
{
  _mm_store_ps(__p, _mm_shuffle_ps(__a, __a, _MM_SHUFFLE(0, 0, 0, 0)));
}
#define _mm_store1_ps _mm_store_ps1

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ss(float *__p, __m128 __a)
{
    struct __unaligned {
      float __v;
    } __attribute__((__packed__, __may_alias__));
    ((struct __unaligned *)__p)->__v = ((__f32x4)__a)[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_ps(float *__p, __m128 __a)
{
  struct __unaligned {
    __m128 __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __unaligned *)__p)->__v = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si16(void *__p, __m128i __a)
{
  struct __unaligned {
    unsigned short __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __unaligned *)__p)->__u = ((__u16x8)__a)[0];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si64(void *__p, __m128i __a)
{
  struct __unaligned {
    unsigned long long __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __unaligned *)__p)->__u = ((__u64x2)__a)[0];
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_ps(__m128 __a)
{
  // TODO: Use .bitmask instruction when available:
  // https://github.com/WebAssembly/simd/pull/201
  union {
    __m128 __v;
    unsigned int __x[4];
  } __attribute__((__packed__, __may_alias__)) __p;
  __p.__v = __a;
  return (__p.__x[0] >> 31)
    | ((__p.__x[1] >> 30) & 2)
    | ((__p.__x[2] >> 29) & 4)
    | ((__p.__x[3] >> 28) & 8);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_move_ss(__m128 __a, __m128 __b)
{
  return (__m128)__f32x4_shuffle(__a, __b, 4, 1, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_add_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_add((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_add_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_add_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sub_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_sub((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sub_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_sub_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_mul_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_mul((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_mul_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_mul_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_div_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_div((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_div_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_div_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_min_ps(__m128 __a, __m128 __b)
{
//  return (__m128)wasm_f32x4_pmin((v128_t)__a, (v128_t)__b); // TODO: Migrate to this, once it works in VMs
  return (__m128)wasm_v128_bitselect((v128_t)__a, (v128_t)__b, (v128_t)wasm_f32x4_lt((v128_t)__a, (v128_t)__b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_min_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_min_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_max_ps(__m128 __a, __m128 __b)
{
//  return (__m128)wasm_f32x4_pmax((v128_t)__a, (v128_t)__b); // TODO: Migrate to this, once it works in VMs
  return (__m128)wasm_v128_bitselect((v128_t)__a, (v128_t)__b, (v128_t)wasm_f32x4_gt((v128_t)__a, (v128_t)__b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_max_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_max_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp_ps(__m128 __a)
{
    return (__m128)wasm_f32x4_div((v128_t)_mm_set1_ps(1.0f), (v128_t)__a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp_ss(__m128 __a)
{
  return _mm_move_ss(__a, _mm_rcp_ps(__a));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_ps(__m128 __a)
{
  return (__m128)wasm_f32x4_sqrt((v128_t)__a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_ss(__m128 __a)
{
  return _mm_move_ss(__a, _mm_sqrt_ps(__a));
}

#define _mm_rsqrt_ps(__a) _mm_rcp_ps(_mm_sqrt_ps((__a)))

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt_ss(__m128 __a)
{
  return _mm_move_ss(__a, _mm_rsqrt_ps(__a));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_ps(__m128 __a, __m128 __b)
{
  return (__m128)__f32x4_shuffle(__a, __b, 2, 6, 3, 7);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_ps(__m128 __a, __m128 __b)
{
  return (__m128)__f32x4_shuffle(__a, __b, 0, 4, 1, 5);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movehl_ps(__m128 __a, __m128 __b)
{
  return (__m128)__f32x4_shuffle(__a, __b, 6, 7, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movelh_ps(__m128 __a, __m128 __b)
{
  return (__m128)__f32x4_shuffle(__a, __b, 0, 1, 4, 5);
}

#define _MM_TRANSPOSE4_PS(row0, row1, row2, row3) \
  do { \
    __m128 __row0 = (row0); \
    __m128 __row1 = (row1); \
    __m128 __row2 = (row2); \
    __m128 __row3 = (row3); \
    __m128 __tmp0 = _mm_unpacklo_ps(__row0, __row1); \
    __m128 __tmp1 = _mm_unpackhi_ps(__row0, __row1); \
    __m128 __tmp2 = _mm_unpacklo_ps(__row2, __row3); \
    __m128 __tmp3 = _mm_unpackhi_ps(__row2, __row3); \
    (row0) = _mm_movelh_ps(__tmp0, __tmp2); \
    (row1) = _mm_movehl_ps(__tmp2, __tmp0); \
    (row2) = _mm_movelh_ps(__tmp1, __tmp3); \
    (row3) = _mm_movehl_ps(__tmp3, __tmp1); \
  } while (0)

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_lt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmplt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmple_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_le((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmple_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmple_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_eq((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpeq_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_ge((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpge_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_gt((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpgt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpord_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_and(wasm_f32x4_eq((v128_t)__a, (v128_t)__a),
                               wasm_f32x4_eq((v128_t)__b, (v128_t)__b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpord_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpord_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpunord_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_or(wasm_f32x4_ne((v128_t)__a, (v128_t)__a),
                              wasm_f32x4_ne((v128_t)__b, (v128_t)__b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpunord_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpunord_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_and_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_and((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_andnot_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_andnot((v128_t)__b, (v128_t)__a);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_or_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_or((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_xor_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_xor((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_f32x4_ne((v128_t)__a, (v128_t)__b);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpneq_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_not((v128_t)_mm_cmpge_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpnge_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_not((v128_t)_mm_cmpgt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpngt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_not((v128_t)_mm_cmple_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpnle_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_ps(__m128 __a, __m128 __b)
{
  return (__m128)wasm_v128_not((v128_t)_mm_cmplt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpnlt_ps(__a, __b));
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comieq_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) == wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comige_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) >= wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comigt_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) > wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comile_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) <= wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comilt_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) < wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comineq_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) != wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomieq_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) == wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomige_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) >= wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomigt_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) > wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomile_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) <= wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomilt_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) < wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomineq_ss(__m128 __a, __m128 __b)
{
  return wasm_f32x4_extract_lane(__a, 0) != wasm_f32x4_extract_lane(__b, 0);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_ss(__m128 __a, int __b)
{
  __f32x4 __v = (__f32x4)__a;
  __v[0] = (float)__b;
  return (__m128)__v;
}
#define _mm_cvt_si2ss _mm_cvtsi32_ss

static __inline__ int __attribute__((__always_inline__, __nodebug__)) _mm_cvtss_si32(__m128 __a)
{
  int x = lrint(((__f32x4)__a)[0]);
  if (x != 0 || fabsf(((__f32x4)__a)[0]) < 2.f)
    return x;
  else
    return (int)0x80000000;
}
#define _mm_cvt_ss2si _mm_cvtss_si32

static __inline__ int __attribute__((__always_inline__, __nodebug__)) _mm_cvttss_si32(__m128 __a)
{
  int x = lrint(((__f32x4)__a)[0]);
  if (x != 0 || fabsf(((__f32x4)__a)[0]) < 2.f)
    return (int)((__f32x4)__a)[0];
  else
    return (int)0x80000000;
}
#define _mm_cvtt_ss2si _mm_cvttss_si32

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_ss(__m128 __a, long long __b)
{
  __f32x4 __v = (__f32x4)__a;
  __v[0] = (float)__b;
  return (__m128)__v;
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_si64(__m128 __a)
{
  if (isnan(((__f32x4)__a)[0]) || isinf(((__f32x4)__a)[0])) return 0x8000000000000000LL;
  long long x = llrint(((__f32x4)__a)[0]);
  if (x != 0xFFFFFFFF00000000ULL && (x != 0 || fabsf(((__f32x4)__a)[0]) < 2.f))
    return x;
  else
    return 0x8000000000000000LL;
}

static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvttss_si64(__m128 __a)
{
  if (isnan(((__f32x4)__a)[0]) || isinf(((__f32x4)__a)[0])) return 0x8000000000000000LL;
  long long x = llrint(((__f32x4)__a)[0]);
  if (x != 0xFFFFFFFF00000000ULL && (x != 0 || fabsf(((__f32x4)__a)[0]) < 2.f))
    return (long long)((__f32x4)__a)[0];
  else
    return 0x8000000000000000LL;
}

static __inline__ float __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_f32(__m128 __a)
{
  return (float)((__f32x4)__a)[0];
}

#define _mm_malloc(__size, __align) memalign((__align), (__size))
#define _mm_free free

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_undefined()
{
  __m128 val;
  return val;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_undefined_ps()
{
  __m128 val;
  return val;
}

#define _MM_EXCEPT_MASK       0x003f
#define _MM_EXCEPT_INVALID    0x0001
#define _MM_EXCEPT_DENORM     0x0002
#define _MM_EXCEPT_DIV_ZERO   0x0004
#define _MM_EXCEPT_OVERFLOW   0x0008
#define _MM_EXCEPT_UNDERFLOW  0x0010
#define _MM_EXCEPT_INEXACT    0x0020

#define _MM_MASK_MASK         0x1f80
#define _MM_MASK_INVALID      0x0080
#define _MM_MASK_DENORM       0x0100
#define _MM_MASK_DIV_ZERO     0x0200
#define _MM_MASK_OVERFLOW     0x0400
#define _MM_MASK_UNDERFLOW    0x0800
#define _MM_MASK_INEXACT      0x1000

#define _MM_ROUND_MASK        0x6000
#define _MM_ROUND_NEAREST     0x0000
#define _MM_ROUND_DOWN        0x2000
#define _MM_ROUND_UP          0x4000
#define _MM_ROUND_TOWARD_ZERO 0x6000

#define _MM_FLUSH_ZERO_MASK   0x8000
#define _MM_FLUSH_ZERO_ON     0x8000
#define _MM_FLUSH_ZERO_OFF    0x0000

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_getcsr()
{
  return _MM_MASK_INEXACT | _MM_MASK_DENORM | _MM_MASK_DIV_ZERO | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INVALID
    | _MM_ROUND_NEAREST | _MM_FLUSH_ZERO_OFF;
}

#define _MM_GET_EXCEPTION_STATE() (_mm_getcsr() & _MM_EXCEPT_MASK)
#define _MM_GET_EXCEPTION_MASK() (_mm_getcsr() & _MM_MASK_MASK)
#define _MM_GET_ROUNDING_MODE() (_mm_getcsr() & _MM_ROUND_MASK)
#define _MM_GET_FLUSH_ZERO_MODE() (_mm_getcsr() & _MM_FLUSH_ZERO_MASK)

// Unavailable functions:
// void _MM_SET_EXCEPTION_STATE(unsigned int __a);
// void _MM_SET_EXCEPTION_MASK(unsigned int __a);
// void _MM_GET_ROUNDING_MODE(unsigned int __a);
// void _MM_GET_FLUSH_ZERO_MODE(unsigned int __a);

#endif /* __emscripten_xmmintrin_h__ */
