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
_mm_setzero_ps(void)
{
  return (__m128){ 0.0, 0.0, 0.0, 0.0 };
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
  return emscripten_float32x4_min(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_max_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_max(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_sqrt_ps(__m128 a)
{
  return emscripten_float32x4_sqrt(a);
}

/* TODO: shuffles */

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
_mm_and_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_and(a, b);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_andnot_ps(__m128 a, __m128 b)
{
  return emscripten_float32x4_andNot(a, b);
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
