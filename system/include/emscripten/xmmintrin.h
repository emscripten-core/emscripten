#include <vector.h>

typedef float32x4 __m128;

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps(float z, float y, float x, float w)
{
  return (__m128){ w, x, y, z };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set1_ps(float w)
{
  return (__m128){ w, w, w, w };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_set_ps1(float w)
{
  return _mm_set1_ps(w);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_setzero_ps(void)
{
  return (__m128){ 0.0, 0.0, 0.0, 0.0 };
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_load_ps(const float *__P)
{
  return *(__m128 *)__P;
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_ps(float *p, __m128 a)
{
  *(__m128 *)p = a;
}

static __inline__ int __attribute__((__always_inline__))
_mm_movemask_ps(__m128 a)
{
  return emscripten_float32x4_signmask(a);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_add_ps(__m128 a, __m128 b)
{
  return a + b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sub_ps(__m128 a, __m128 b)
{
  return a - b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_mul_ps(__m128 a, __m128 b)
{
  return a * b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_div_ps(__m128 a, __m128 b)
{
  return a / b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_min_ps(__m128 a, __m128 b)
{
  // Use a comparsion and select instead of emscripten_float32x4_min in order to
  // correctly emulate x86's NaN and -0.0 semantics.
  return emscripten_float32x4_select(emscripten_float32x4_lessThan(a, b), a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ps(__m128 a, __m128 b)
{
  // Use a comparsion and select instead of emscripten_float32x4_max in order to
  // correctly emulate x86's NaN and -0.0 semantics.
  return emscripten_float32x4_select(emscripten_float32x4_greaterThan(a, b), a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ps(__m128 a)
{
  return emscripten_float32x4_sqrt(a);
}

/* TODO: more shuffles */

// This is defined as a macro because __builtin_shufflevector requires its
// mask argument to be a compile-time constant.
#define _mm_shuffle_ps(a, b, mask) __extension__ ({ \
  __m128 __a = (a); \
  __m128 __b = (b); \
  (__m128)__builtin_shufflevector(__a, __b, \
                                 (((mask) >> 0) & 0x3) + 0, \
                                 (((mask) >> 2) & 0x3) + 0, \
                                 (((mask) >> 4) & 0x3) + 4, \
                                 (((mask) >> 6) & 0x3) + 4); \
})

#define _MM_SHUFFLE(w, z, y, x) (((w) << 6) | ((z) << 4) | ((y) << 2) | (x))

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmplt_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_lessThan(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmple_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_lessThanOrEqual(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpeq_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_equal(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpge_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_greaterThanOrEqual(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpgt_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_greaterThan(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnlt_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_not(emscripten_float32x4_lessThan(a, b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cmpnle_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_not(emscripten_float32x4_lessThanOrEqual(a, b));
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_and_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_and(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_andnot_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_and(emscripten_float32x4_not(a), b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_or_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_or(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_xor_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_xor(a, b);
}
