#ifndef __emscripten_xmmintrin_h__
#define __emscripten_xmmintrin_h__

#include <vector.h>

// Emscripten SIMD support doesn't support MMX/float32x2/__m64.
// However, we support loading and storing 2-vectors, so
// treat "__m64 *" as "void *" for that purpose.
typedef void __m64;

typedef float32x4 __m128;
typedef int32x4 __v4si;
typedef float32x4 __v4sf;

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps(float __z, float __y, float __x, float __w)
{
  return (__m128){ __w, __x, __y, __z };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setr_ps(float __z, float __y, float __x, float __w)
{
  return (__m128){ __z, __y, __x, __w };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ss(float __w)
{
  return (__m128){ __w, 0, 0, 0 };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps1(float __w)
{
  return (__m128){ __w, __w, __w, __w };
}

#define _mm_set1_ps _mm_set_ps1

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setzero_ps(void)
{
  return (__m128){ 0.0f, 0.0f, 0.0f, 0.0f };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ps(const float *__p)
{
  return *(__m128 *)__p;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_loadl_pi(__m128 __a, const void /*__m64*/ *__p)
{
  return (__m128){ ((const float*)__p)[0], ((const float*)__p)[1], __a[2], __a[3] };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_loadh_pi(__m128 __a, const void /*__m64*/ *__p)
{
  return (__m128){ __a[0], __a[1], ((const float*)__p)[0], ((const float*)__p)[1] };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_loadr_ps(const float *__p)
{
  return (__m128){ ((const float*)__p)[3], ((const float*)__p)[2], ((const float*)__p)[1], ((const float*)__p)[0] };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_loadu_ps(const float *__p)
{
  struct __unaligned {
    __m128 __v;
  } __attribute__((__packed__, __may_alias__));

  return ((struct __unaligned *)__p)->__v;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ps1(const float *__p)
{
  return (__m128){ *__p, *__p, *__p, *__p };
}
#define _mm_load1_ps _mm_load_ps1

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ss(const float *__p)
{
  // TODO: This actually corresponds to the SIMD.float32x4.loadX function in SIMD.js. Use that instead.
  return (__m128){ *__p, 0.0f, 0.0f, 0.0f };
}

static __inline__ void __attribute__((__always_inline__))
_mm_storel_pi(void /*__m64*/ *__p, __m128 __a)
{
  ((float*)__p)[0] = __a[0];
  ((float*)__p)[1] = __a[1];
}

static __inline__ void __attribute__((__always_inline__))
_mm_storeh_pi(void /*__m64*/ *__p, __m128 __a)
{
  ((float*)__p)[0] = __a[2];
  ((float*)__p)[1] = __a[3];
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ps(float *__p, __m128 __a)
{
  *(__m128 *)__p = __a;
}
// No NTA cache hint available.
#define _mm_stream_ps _mm_store_ps

#define _MM_SHUFFLE(w, z, y, x) (((w) << 6) | ((z) << 4) | ((y) << 2) | (x))

// This is defined as a macro because __builtin_shufflevector requires its
// mask argument to be a compile-time constant.
#define _mm_shuffle_ps(a, b, mask) \
  ((__m128)__builtin_shufflevector((a), (b), \
                                  (((mask) >> 0) & 0x3) + 0, \
                                  (((mask) >> 2) & 0x3) + 0, \
                                  (((mask) >> 4) & 0x3) + 4, \
                                  (((mask) >> 6) & 0x3) + 4))

static __inline__ void __attribute__((__always_inline__))
_mm_storer_ps(float *__p, __m128 __a)
{
  _mm_store_ps(__p, _mm_shuffle_ps(__a, __a, _MM_SHUFFLE(0, 1, 2, 3)));
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ps1(float *__p, __m128 __a)
{
  _mm_store_ps(__p, _mm_shuffle_ps(__a, __a, _MM_SHUFFLE(0, 0, 0, 0)));
}
#define _mm_store1_ps _mm_store_ps1

static __inline__ void __attribute__((__always_inline__))
_mm_store_ss(float *__p, __m128 __a)
{
  *__p = __a[0];
}

static __inline__ void __attribute__((__always_inline__))
_mm_storeu_ps(float *__p, __m128 __a)
{
  struct __unaligned {
    __m128 __v;
  } __attribute__((__packed__, __may_alias__));

  ((struct __unaligned *)__p)->__v = __a;
}

static __inline__ int __attribute__((__always_inline__))
_mm_movemask_ps(__m128 __a)
{
  return emscripten_float32x4_signmask(__a);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_move_ss(__m128 __a, __m128 __b)
{
  return __builtin_shufflevector(__a, __b, 4, 1, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_add_ps(__m128 __a, __m128 __b)
{
  return __a + __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_add_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_add_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sub_ps(__m128 __a, __m128 __b)
{
  return __a - __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sub_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_sub_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_mul_ps(__m128 __a, __m128 __b)
{
  return __a * __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_mul_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_mul_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_div_ps(__m128 __a, __m128 __b)
{
  return __a / __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_div_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_div_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_min_ps(__m128 __a, __m128 __b)
{
  // Use a comparison and select instead of emscripten_float32x4_min in order to
  // correctly emulate x86's NaN and -0.0 semantics.
  return emscripten_float32x4_select(emscripten_float32x4_lessThan(__a, __b), __a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_min_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_min_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ps(__m128 __a, __m128 __b)
{
  // Use a comparison and select instead of emscripten_float32x4_max in order to
  // correctly emulate x86's NaN and -0.0 semantics.
  return emscripten_float32x4_select(emscripten_float32x4_greaterThan(__a, __b), __a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_max_ps(__a, __b));
}

// TODO: we should re-evaluate rcpps, rsqrtps, and friends once we figure out what we're doing with SIMD.float32x4.reciprocal, SIMD.float32x4.reciprocalSqrt, and friends in SIMD.js.
#define _mm_rcp_ps(__a) (_mm_set1_ps(1.0f) / (__a))

static __inline__ __m128 __attribute__((__always_inline__))
_mm_rcp_ss(__m128 __a)
{
  return _mm_move_ss(__a, _mm_rcp_ps(__a));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ps(__m128 __a)
{
  return emscripten_float32x4_sqrt(__a);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ss(__m128 __a)
{
  return _mm_move_ss(__a, _mm_sqrt_ps(__a));
}

#define _mm_rsqrt_ps(__a) _mm_rcp_ps(_mm_sqrt_ps((__a)))

static __inline__ __m128 __attribute__((__always_inline__))
_mm_rsqrt_ss(__m128 __a)
{
  return _mm_move_ss(__a, _mm_rsqrt_ps(__a));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_unpackhi_ps(__m128 __a, __m128 __b)
{
  return __builtin_shufflevector(__a, __b, 2, 6, 3, 7);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_unpacklo_ps(__m128 __a, __m128 __b)
{
  return __builtin_shufflevector(__a, __b, 0, 4, 1, 5);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_movehl_ps(__m128 __a, __m128 __b)
{
  return __builtin_shufflevector(__a, __b, 6, 7, 2, 3);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_movelh_ps(__m128 __a, __m128 __b)
{
  return __builtin_shufflevector(__a, __b, 0, 1, 4, 5);
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

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmplt_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_lessThan(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmplt_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmplt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmple_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_lessThanOrEqual(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmple_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmple_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpeq_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_equal(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpeq_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpeq_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpge_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_greaterThanOrEqual(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpge_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpge_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpgt_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_greaterThan(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpgt_ss(__m128 __a, __m128 __b)
{
  return _mm_move_ss(__a, _mm_cmpgt_ps(__a, __b));
}

static __inline__ int __internal_isnan(float __f)
{
  return (*(unsigned int*)&__f << 1) > 0xFF000000u;
}

static __inline__ __m128 __attribute__((__always_inline__)) _mm_cmpord_ps(__m128 __a, __m128 __b)
{
  unsigned int r[4];
  r[0] = (!__internal_isnan(__a[0]) && !__internal_isnan(__b[0])) ? 0xFFFFFFFFU : 0;
  r[1] = (!__internal_isnan(__a[1]) && !__internal_isnan(__b[1])) ? 0xFFFFFFFFU : 0;
  r[2] = (!__internal_isnan(__a[2]) && !__internal_isnan(__b[2])) ? 0xFFFFFFFFU : 0;
  r[3] = (!__internal_isnan(__a[3]) && !__internal_isnan(__b[3])) ? 0xFFFFFFFFU : 0;
  return _mm_loadu_ps((float*)r);
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpord_ss(__m128 __a, __m128 __b)
{
  unsigned int r = (!__internal_isnan(__a[0]) && !__internal_isnan(__b[0])) ? 0xFFFFFFFFU : 0;
  return _mm_move_ss(__a, _mm_set_ss(*(float*)&r));
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpunord_ps(__m128 __a, __m128 __b)
{
  union {
    unsigned int r[4];
    __m128 m;
  } u;
  u.r[0] = (__internal_isnan(__a[0]) || __internal_isnan(__b[0])) ? 0xFFFFFFFFU : 0;
  u.r[1] = (__internal_isnan(__a[1]) || __internal_isnan(__b[1])) ? 0xFFFFFFFFU : 0;
  u.r[2] = (__internal_isnan(__a[2]) || __internal_isnan(__b[2])) ? 0xFFFFFFFFU : 0;
  u.r[3] = (__internal_isnan(__a[3]) || __internal_isnan(__b[3])) ? 0xFFFFFFFFU : 0;
  return u.m;
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__)) _mm_cmpunord_ss(__m128 __a, __m128 __b)
{
  unsigned int r = (__internal_isnan(__a[0]) || __internal_isnan(__b[0])) ? 0xFFFFFFFFU : 0;
  return _mm_move_ss(__a, _mm_set_ss(*(float*)&r));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_and_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_and(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_andnot_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_and(emscripten_float32x4_not(__a), __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_or_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_or(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_xor_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_xor(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
__internal_not_ss(__m128 __a)
{
  unsigned int __x = 0xFFFFFFFFU;
  return _mm_xor_ps(__a, _mm_set_ss(*(float*)&__x));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpneq_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(_mm_cmpeq_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpneq_ss(__m128 __a, __m128 __b)
{
  return __internal_not_ss(_mm_cmpeq_ss(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnge_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(_mm_cmpge_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnge_ss(__m128 __a, __m128 __b)
{
  return __internal_not_ss(_mm_cmpge_ss(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpngt_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(_mm_cmpgt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpngt_ss(__m128 __a, __m128 __b)
{
  return __internal_not_ss(_mm_cmpgt_ss(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnle_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(_mm_cmple_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnle_ss(__m128 __a, __m128 __b)
{
  return __internal_not_ss(_mm_cmple_ss(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnlt_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(_mm_cmplt_ps(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnlt_ss(__m128 __a, __m128 __b)
{
  return __internal_not_ss(_mm_cmplt_ss(__a, __b));
}

static __inline__ int __attribute__((__always_inline__))
_mm_comieq_ss(__m128 __a, __m128 __b)
{
  return __a[0] == __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_comige_ss(__m128 __a, __m128 __b)
{
  return __a[0] >= __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_comigt_ss(__m128 __a, __m128 __b)
{
  return __a[0] > __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_comile_ss(__m128 __a, __m128 __b)
{
  return __a[0] <= __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_comilt_ss(__m128 __a, __m128 __b)
{
  return __a[0] < __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_comineq_ss(__m128 __a, __m128 __b)
{
  return __a[0] != __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_ucomieq_ss(__m128 __a, __m128 __b)
{
  return __a[0] == __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_ucomige_ss(__m128 __a, __m128 __b)
{
  return __a[0] >= __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_ucomigt_ss(__m128 __a, __m128 __b)
{
  return __a[0] > __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_ucomile_ss(__m128 __a, __m128 __b)
{
  return __a[0] <= __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_ucomilt_ss(__m128 __a, __m128 __b)
{
  return __a[0] < __b[0];
}

static __inline__ int __attribute__((__always_inline__))
_mm_ucomineq_ss(__m128 __a, __m128 __b)
{
  return __a[0] != __b[0];
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cvtsi32_ss(__m128 __a, int __b)
{
  __a[0] = (float)__b;
  return __a;
}
#define _mm_cvt_si2ss _mm_cvtsi32_ss

static __inline__ int __attribute__((__always_inline__)) _mm_cvtss_si32(__m128 a)
{
  return (int)a[0]; // TODO: Rounding mode
}
#define _mm_cvt_ss2si _mm_cvtss_si32

static __inline__ int __attribute__((__always_inline__)) _mm_cvttss_si32(__m128 a)
{
  return (int)a[0]; // TODO: Rounding mode, truncate.
}
#define _mm_cvtt_ss2si _mm_cvttss_si32

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cvtsi64_ss(__m128 __a, long long __b)
{
  __a[0] = (float)__b;
  return __a;
}

static __inline__ long long __attribute__((__always_inline__))
_mm_cvtss_si64(__m128 __a)
{
  return (long long)__a[0]; // TODO: Rounding mode
}

static __inline__ long long __attribute__((__always_inline__))
_mm_cvttss_si64(__m128 __a)
{
  return (long long)__a[0]; // TODO: Rounding mode, truncate.
}

static __inline__ float __attribute__((__always_inline__))
_mm_cvtss_f32(__m128 __a)
{
  return (float)__a[0];
}
#endif
