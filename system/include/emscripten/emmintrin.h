#include <xmmintrin.h>

typedef int32x4 __m128i;

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set_epi32(int z, int y, int x, int w)
{
  return (__m128i){ w, x, y, z };
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_set1_epi32(int w)
{
  return (__m128i){ w, w, w, w };
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_setzero_si128()
{
  return (__m128i){ 0, 0, 0, 0 };
}

static __inline__ void __attribute__((__always_inline__))
_mm_store_si128(__m128i *p, __m128i a)
{
  *p = a;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_and_si128(__m128i a, __m128i b)
{
  return a & b;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_andnot_si128(__m128i a, __m128i b)
{
  return ~a & b;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_or_si128(__m128i a, __m128i b)
{
  return a | b;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_xor_si128(__m128i a, __m128i b)
{
  return a ^ b;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_add_epi32(__m128i a, __m128i b)
{
  return a + b;
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_sub_epi32(__m128i a, __m128i b)
{
  return a - b;
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_castsi128_ps(__m128i a)
{
  return emscripten_float32x4_fromInt32x4Bits(a);
}

static __inline__ __m128 __attribute__((__always_inline__))
_mm_cvtepi32_ps(__m128i a)
{
  return emscripten_float32x4_fromInt32x4(a);
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_castps_si128(__m128 a)
{
  return emscripten_int32x4_fromFloat32x4Bits(a);
}

static __inline__ __m128i __attribute__((__always_inline__))
_mm_cvtps_epi32(__m128 a)
{
  return emscripten_int32x4_fromFloat32x4(a);
}