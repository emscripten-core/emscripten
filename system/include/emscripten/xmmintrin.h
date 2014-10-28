#ifndef __emscripten_xmmintrin_h__
#define __emscripten_xmmintrin_h__

#include <vector.h>

typedef float32x4 __m128;
typedef int32x4 __v4si;
typedef float32x4 __v4sf;

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps(float __z, float __y, float __x, float __w)
{
  return (__m128){ __w, __x, __y, __z };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set1_ps(float __w)
{
  return (__m128){ __w, __w, __w, __w };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps1(float __w)
{
  return _mm_set1_ps(__w);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setzero_ps(void)
{
  return (__m128){ 0.0, 0.0, 0.0, 0.0 };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ps(const float *__p)
{
  return *(__m128 *)__p;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_loadu_ps(const float *__p)
{
  struct __unaligned {
    __m128 __v;
  } __attribute__((__packed__, __may_alias__));

  return ((struct __unaligned *)__p)->__v;
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ps(float *__p, __m128 __a)
{
  *(__m128 *)__p = __a;
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
_mm_add_ps(__m128 __a, __m128 __b)
{
  return __a + __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sub_ps(__m128 __a, __m128 __b)
{
  return __a - __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_mul_ps(__m128 __a, __m128 __b)
{
  return __a * __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_div_ps(__m128 __a, __m128 __b)
{
  return __a / __b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_min_ps(__m128 __a, __m128 __b)
{
  // Use a comparsion and select instead of emscripten_float32x4_min in order to
  // correctly emulate x86's NaN and -0.0 semantics.
  return emscripten_float32x4_select(emscripten_float32x4_lessThan(__a, __b), __a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ps(__m128 __a, __m128 __b)
{
  // Use a comparsion and select instead of emscripten_float32x4_max in order to
  // correctly emulate x86's NaN and -0.0 semantics.
  return emscripten_float32x4_select(emscripten_float32x4_greaterThan(__a, __b), __a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ps(__m128 __a)
{
  return emscripten_float32x4_sqrt(__a);
}

// This is defined as a macro because __builtin_shufflevector requires its
// mask argument to be a compile-time constant.
#define _mm_shuffle_ps(a, b, mask) \
  ((__m128)__builtin_shufflevector((a), (b), \
                                  (((mask) >> 0) & 0x3) + 0, \
                                  (((mask) >> 2) & 0x3) + 0, \
                                  (((mask) >> 4) & 0x3) + 4, \
                                  (((mask) >> 6) & 0x3) + 4))

#define _MM_SHUFFLE(w, z, y, x) (((w) << 6) | ((z) << 4) | ((y) << 2) | (x))

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
_mm_move_ss(__m128 __a, __m128 __b)
{
  return __builtin_shufflevector(__a, __b, 4, 1, 2, 3);
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
_mm_cmple_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_lessThanOrEqual(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpeq_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_equal(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpge_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_greaterThanOrEqual(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpgt_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_greaterThan(__a, __b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnlt_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(emscripten_float32x4_lessThan(__a, __b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnle_ps(__m128 __a, __m128 __b)
{
  return emscripten_float32x4_not(emscripten_float32x4_lessThanOrEqual(__a, __b));
}

// TODO: _mm_cmpord_ps, _mm_cmpunord_ps

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

#endif
