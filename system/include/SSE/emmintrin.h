/*===---- emmintrin.h - SSE2 intrinsics ------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __EMMINTRIN_H
#define __EMMINTRIN_H

#ifdef __EMSCRIPTEN__

// XXX TODO: Remove after _mm_sqrt_pd/sd is supported.
#include <math.h>

#include <emscripten/emscripten.h>

#define __SATURATE(x, Min, Max) ((x) >= Min ? ((x) <= Max ? (x) : Max) : Min)
#define __MIN(x, y) ((x) <= (y) ? (x) : (y))
#define __MAX(x, y) ((x) >= (y) ? (x) : (y))

// Alias different (functionally) equivalent intrinsics.
#define _mm_cvtsd_si64x _mm_cvtsd_si64
#define _mm_cvtsi128_si64x _mm_cvtsi128_si64
#define _mm_cvtsi64x_sd _mm_cvtsi64_sd
#define _mm_cvtsi64x_si128 _mm_cvtsi64_si128
#define _mm_cvttsd_si64x _mm_cvttsd_si64
#define _mm_store_pd1 _mm_store1_pd

#endif

#ifndef __SSE2__
#error "SSE2 instruction set not enabled"
#endif

#include <xmmintrin.h>

typedef double __m128d __attribute__((__vector_size__(16)));
#ifdef __EMSCRIPTEN__
typedef int __m128i __attribute__((__vector_size__(16)));
#else
typedef long long __m128i __attribute__((__vector_size__(16)));
#endif

/* Type defines.  */
typedef double __v2df __attribute__ ((__vector_size__ (16)));
typedef long long __v2di __attribute__ ((__vector_size__ (16)));
typedef short __v8hi __attribute__((__vector_size__(16)));
typedef char __v16qi __attribute__((__vector_size__(16)));

/* We need an explicitly signed variant for char. Note that this shouldn't
 * appear in the interface though. */
typedef signed char __v16qs __attribute__((__vector_size__(16)));

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_add_sd(__m128d __a, __m128d __b)
{
  __a[0] += __b[0];
  return __a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_add_pd(__m128d __a, __m128d __b)
{
  return __a + __b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sub_sd(__m128d __a, __m128d __b)
{
  __a[0] -= __b[0];
  return __a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sub_pd(__m128d __a, __m128d __b)
{
  return __a - __b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_mul_sd(__m128d __a, __m128d __b)
{
  __a[0] *= __b[0];
  return __a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_mul_pd(__m128d __a, __m128d __b)
{
  return __a * __b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_div_sd(__m128d __a, __m128d __b)
{
  __a[0] /= __b[0];
  return __a;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_div_pd(__m128d __a, __m128d __b)
{
  return __a / __b;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  // TODO: SIMD.jsize!
  return (__m128d) { sqrt(__b[0]), __a[1] };
#else
  __m128d __c = __builtin_ia32_sqrtsd(__b);
  return (__m128d) { __c[0], __a[1] };
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_pd(__m128d __a)
{
#ifdef __EMSCRIPTEN__
  // TODO: SIMD.jsize!
  return (__m128d) { sqrt(__a[0]), sqrt(__a[1]) };
#else
  return __builtin_ia32_sqrtpd(__a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_move_sd(__m128d __a, __m128d __b)
{
  return (__m128d){ __b[0], __a[1] };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_min_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return (__m128d) { __a[0] < __b[0] ? __a[0] : __b[0],
    __a[1] < __b[1] ? __a[1] : __b[1]
  };

  // The vectorized version in SIMD.js would be the following, but
  // this does not work due to https://bugzilla.mozilla.org/show_bug.cgi?id=1176375.
  // TODO: Remove the above scalarized version once the bug is fixed.

  // Use a comparison and select instead of emscripten_float32x4_min in order to
  // correctly emulate x86's NaN and -0.0 semantics.
 // return emscripten_float64x2_select(emscripten_float64x2_lessThan(__a, __b), __a, __b);
#else
  return __builtin_ia32_minpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_min_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_min_pd(__a, __b));
#else
  return __builtin_ia32_minsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_max_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return (__m128d) { __a[0] > __b[0] ? __a[0] : __b[0],
    __a[1] > __b[1] ? __a[1] : __b[1]
  };

  // The vectorized version in SIMD.js would be the following, but
  // this does not work due to https://bugzilla.mozilla.org/show_bug.cgi?id=1176375.
  // TODO: Remove the above scalarized version once the bug is fixed.

  // Use a comparison and select instead of emscripten_float32x4_max in order to
  // correctly emulate x86's NaN and -0.0 semantics.
//  return emscripten_float64x2_select(emscripten_float64x2_greaterThan(__a, __b), __a, __b);
#else
  return __builtin_ia32_maxpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_max_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_max_pd(__a, __b));
#else
  return __builtin_ia32_maxsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_and_pd(__m128d __a, __m128d __b)
{
  return (__m128d)((__v4si)__a & (__v4si)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_andnot_pd(__m128d __a, __m128d __b)
{
  return (__m128d)(~(__v4si)__a & (__v4si)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_or_pd(__m128d __a, __m128d __b)
{
  return (__m128d)((__v4si)__a | (__v4si)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_xor_pd(__m128d __a, __m128d __b)
{
  return (__m128d)((__v4si)__a ^ (__v4si)__b);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_equal(__a, __b);
#else
  return (__m128d)__builtin_ia32_cmpeqpd(__a, __b);
#endif
}
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_lessThan(__a, __b);
#else
  return (__m128d)__builtin_ia32_cmpltpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmple_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_lessThanOrEqual(__a, __b);
#else
  return (__m128d)__builtin_ia32_cmplepd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_greaterThan(__a, __b);
#else
  return (__m128d)__builtin_ia32_cmpltpd(__b, __a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_greaterThanOrEqual(__a, __b);
#else
  return (__m128d)__builtin_ia32_cmplepd(__b, __a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_and(emscripten_float64x2_equal(__a, __a),
                                  emscripten_float64x2_equal(__b, __b));
#else
  return (__m128d)__builtin_ia32_cmpordpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_or(emscripten_float64x2_notEqual(__a, __a),
                                 emscripten_float64x2_notEqual(__b, __b));
#else
  return (__m128d)__builtin_ia32_cmpunordpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_notEqual(__a, __b);
#else
  return (__m128d)__builtin_ia32_cmpneqpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_not(_mm_cmplt_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpnltpd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_not(_mm_cmple_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpnlepd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_not(_mm_cmpgt_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpnltpd(__b, __a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_pd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float64x2_not(_mm_cmpge_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpnlepd(__b, __a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpeq_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpeqsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmplt_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpltsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmple_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmple_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmplesd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpgt_pd(__a, __b));
#else
  __m128d __c = __builtin_ia32_cmpltsd(__b, __a);
  return (__m128d) { __c[0], __a[1] };
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpge_pd(__a, __b));
#else
  __m128d __c = __builtin_ia32_cmplesd(__b, __a);
  return (__m128d) { __c[0], __a[1] };
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpord_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpordsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpunord_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpunordsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpneq_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpneqsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpnlt_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpnltsd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpnle_pd(__a, __b));
#else
  return (__m128d)__builtin_ia32_cmpnlesd(__a, __b);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpngt_pd(__a, __b));
#else
  __m128d __c = __builtin_ia32_cmpnltsd(__b, __a);
  return (__m128d) { __c[0], __a[1] };
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return _mm_move_sd(__a, _mm_cmpnge_pd(__a, __b));
#else
  __m128d __c = __builtin_ia32_cmpnlesd(__b, __a);
  return (__m128d) { __c[0], __a[1] };
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comieq_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  if (isnan(__a[0]) || isnan(__b[0])) return 1;
  return __a[0] == __b[0];
#else
  return __builtin_ia32_comisdeq(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comilt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  if (isnan(__a[0]) || isnan(__b[0])) return 1;
  return __a[0] < __b[0];
#else
  return __builtin_ia32_comisdlt(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comile_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  if (isnan(__a[0]) || isnan(__b[0])) return 1;
  return __a[0] <= __b[0];
#else
  return __builtin_ia32_comisdle(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comigt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return __a[0] > __b[0];
#else
  return __builtin_ia32_comisdgt(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comige_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return __a[0] >= __b[0];
#else
  return __builtin_ia32_comisdge(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comineq_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  if (isnan(__a[0]) || isnan(__b[0])) return 0;
  return __a[0] != __b[0];
#else
  return __builtin_ia32_comisdneq(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomieq_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  if (isnan(__a[0]) || isnan(__b[0])) return 1;
  return __a[0] == __b[0];
#else
  return __builtin_ia32_ucomisdeq(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomilt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return !(__a[0] >= __b[0]);
#else
  return __builtin_ia32_ucomisdlt(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomile_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return !(__a[0] > __b[0]);
#else
  return __builtin_ia32_ucomisdle(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomigt_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return __a[0] > __b[0];
#else
  return __builtin_ia32_ucomisdgt(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomige_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  return __a[0] >= __b[0];
#else
  return __builtin_ia32_ucomisdge(__a, __b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomineq_sd(__m128d __a, __m128d __b)
{
#ifdef __EMSCRIPTEN__
  if (isnan(__a[0]) || isnan(__b[0])) return 0;
  return __a[0] != __b[0];
#else
  return __builtin_ia32_ucomisdneq(__a, __b);
#endif
}

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_ps(__m128d __a)
{
#ifdef __EMSCRIPTEN__
  return (__m128) { (float)__a[0], (float)__a[1], 0, 0 };
#else
  return __builtin_ia32_cvtpd2ps(__a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_pd(__m128 __a)
{
#ifdef __EMSCRIPTEN__
  return (__m128d) { (double)__a[0], (double)__a[1] };
#else
  return __builtin_ia32_cvtps2pd(__a);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_pd(__m128i __a)
{
#ifdef __EMSCRIPTEN__
  return (__m128d) { (double)__a[0], (double)__a[1] };
#else
  return __builtin_ia32_cvtdq2pd((__v4si)__a);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_epi32(__m128d __a)
{
#ifdef __EMSCRIPTEN__
  int m[2];
  for(int i = 0; i < 2; ++i)
  {
    int x = lrint(__a[i]);
    if (x != 0 || fabs(__a[i]) < 2.0)
      m[i] = (int)x;
    else
      m[i] = (int)0x80000000;
  }
  return (__m128i) { m[0], m[1], 0, 0 };
#else
  return __builtin_ia32_cvtpd2dq(__a);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si32(__m128d __a)
{
#ifdef __EMSCRIPTEN__
  int x = lrint(__a[0]);
  if (x != 0 || fabs(__a[0]) < 2.0)
    return (int)x;
  else
    return (int)0x80000000;
#else
  return __builtin_ia32_cvtsd2si(__a);
#endif
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
#ifdef __EMSCRIPTEN__
  int m[2];
  for(int i = 0; i < 2; ++i)
  {
    int x = lrint(__a[i]);
    if (x != 0 || fabs(__a[i]) < 2.0)
      m[i] = (int)__a[i];
    else
      m[i] = (int)0x80000000;
  }
  return (__m128i) { m[0], m[1], 0, 0 };
#else
  return (__m128i)__builtin_ia32_cvttpd2dq(__a);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si32(__m128d __a)
{
  int x = lrint(__a[0]);
  if (x != 0 || fabs(__a[0]) < 2.0)
    return (int)__a[0];
  else
    return (int)0x80000000;
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvtpd_pi32(__m128d __a)
{
  return (__m64)__builtin_ia32_cvtpd2pi(__a);
}

static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_cvttpd_pi32(__m128d __a)
{
  return (__m64)__builtin_ia32_cvttpd2pi(__a);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtpi32_pd(__m64 __a)
{
  return __builtin_ia32_cvtpi2pd((__v2si)__a);
}
#endif

static __inline__ double __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_f64(__m128d __a)
{
  return __a[0];
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load_pd(double const *__dp)
{
  return *(__m128d*)__dp;
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_load1_pd(double const *__dp)
{
  struct __mm_load1_pd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  double __u = ((struct __mm_load1_pd_struct*)__dp)->__u;
  return (__m128d){ __u, __u };
}

#define        _mm_load_pd1(dp)        _mm_load1_pd(dp)

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadr_pd(double const *__dp)
{
  __m128d __u = *(__m128d*)__dp;
  return __builtin_shufflevector(__u, __u, 1, 0);
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
_mm_load_sd(double const *__dp)
{
  struct __mm_load_sd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  double __u = ((struct __mm_load_sd_struct*)__dp)->__u;
  return (__m128d){ __u, 0 };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadh_pd(__m128d __a, double const *__dp)
{
  struct __mm_loadh_pd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  double __u = ((struct __mm_loadh_pd_struct*)__dp)->__u;
  return (__m128d){ __a[0], __u };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_loadl_pd(__m128d __a, double const *__dp)
{
  struct __mm_loadl_pd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  double __u = ((struct __mm_loadl_pd_struct*)__dp)->__u;
  return (__m128d){ __u, __a[1] };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set_sd(double __w)
{
  return (__m128d){ __w, 0 };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set1_pd(double __w)
{
  return (__m128d){ __w, __w };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_set_pd(double __w, double __x)
{
  return (__m128d){ __x, __w };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_setr_pd(double __w, double __x)
{
  return (__m128d){ __w, __x };
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_setzero_pd(void)
{
  return (__m128d){ 0, 0 };
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_sd(double *__dp, __m128d __a)
{
  struct __mm_store_sd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_store_sd_struct*)__dp)->__u = __a[0];
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
#ifdef __EMSCRIPTEN__
  struct __unaligned {
    __m128d __v;
  } __attribute__((__packed__, __may_alias__));

  ((struct __unaligned *)__dp)->__v = __a;
#else
  __builtin_ia32_storeupd(__dp, __a);
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storer_pd(double *__dp, __m128d __a)
{
  __a = __builtin_shufflevector(__a, __a, 1, 0);
  *(__m128d *)__dp = __a;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeh_pd(double *__dp, __m128d __a)
{
  struct __mm_storeh_pd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_storeh_pd_struct*)__dp)->__u = __a[1];
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_pd(double *__dp, __m128d __a)
{
  struct __mm_storeh_pd_struct {
    double __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_storeh_pd_struct*)__dp)->__u = __a[0];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)((__v16qi)__a + (__v16qi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)((__v8hi)__a + (__v8hi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)((__v4si)__a + (__v4si)__b);
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_add_si64(__m64 __a, __m64 __b)
{
  return __a + __b;
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_add_epi64(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    long long x[2];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  dst.x[0] = src.x[0] + src2.x[0];
  dst.x[1] = src.x[1] + src2.x[1];
  return dst.m;
#else
  return __a + __b;
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epi8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = __SATURATE(src.x[i] + src2.x[i], -128, 127);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_paddsb128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = __SATURATE(src.x[i] + src2.x[i], -32768, 32767);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_paddsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epu8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = __SATURATE(src.x[i] + src2.x[i], 0, 255);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_paddusb128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_adds_epu16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = __SATURATE(src.x[i] + src2.x[i], 0, 65535);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_paddusw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_avg_epu8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = (src.x[i] + src2.x[i] + 1) >> 1;
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pavgb128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_avg_epu16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = (src.x[i] + src2.x[i] + 1) >> 1;
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pavgw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_madd_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2;
  union {
    signed int x[4];
    __m128i m;
  } dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 4; ++i)
    dst.x[i] = src.x[i*2] * src2.x[i*2] + src.x[i*2+1] * src2.x[i*2+1];
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pmaddwd128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = __MAX(src.x[i], src2.x[i]);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pmaxsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_max_epu8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = __MAX(src.x[i], src2.x[i]);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pmaxub128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = __MIN(src.x[i], src2.x[i]);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pminsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_min_epu8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = __MIN(src.x[i], src2.x[i]);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pminub128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = (signed short)(((int)src.x[i] * (int)src2.x[i]) >> 16);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pmulhw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhi_epu16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = (unsigned short)(((int)src.x[i] * (int)src2.x[i]) >> 16);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_pmulhuw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mullo_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)((__v8hi)__a * (__v8hi)__b);
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_mul_su32(__m64 __a, __m64 __b)
{
  return __builtin_ia32_pmuludq((__v2si)__a, (__v2si)__b);
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epu32(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  unsigned long long a0 = (unsigned long long)(unsigned int)__a[0];
  unsigned long long a2 = (unsigned long long)(unsigned int)__a[2];
  unsigned long long b0 = (unsigned long long)(unsigned int)__b[0];
  unsigned long long b2 = (unsigned long long)(unsigned int)__b[2];
  union {
    unsigned long long x[2];
    __m128i m;
  } u;
  u.x[0] = a0*b0;
  u.x[1] = a2*b2;
  return u.m;
#else
  return __builtin_ia32_pmuludq128((__v4si)__a, (__v4si)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sad_epu8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src, src2;
  src.m = __a;
  src2.m = __b;
  union {
    unsigned short x[8];
    __m128i m;
  } dst;
#define __ABS(a) ((a) < 0 ? -(a) : (a))
  for(int i = 0; i < 8; ++i)
    dst.x[i] = 0;
  for(int i = 0; i < 8; ++i)
  {
    dst.x[0] += __ABS(src.x[i] - src2.x[i]);
    dst.x[4] += __ABS(src.x[8+i] - src2.x[8+i]);
  }
  return dst.m;
#undef __ABS
#else
  return __builtin_ia32_psadbw128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)((__v16qi)__a - (__v16qi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)((__v8hi)__a - (__v8hi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)((__v4si)__a - (__v4si)__b);
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_sub_si64(__m64 __a, __m64 __b)
{
  return __a - __b;
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sub_epi64(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    long long x[2];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  dst.x[0] = src.x[0] - src2.x[0];
  dst.x[1] = src.x[1] - src2.x[1];
  return dst.m;
#else
  return __a - __b;
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epi8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = __SATURATE(src.x[i] - src2.x[i], -128, 127);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_psubsb128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = __SATURATE(src.x[i] - src2.x[i], -32768, 32767);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_psubsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epu8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 16; ++i)
    dst.x[i] = __SATURATE(src.x[i] - src2.x[i], 0, 255);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_psubusb128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_subs_epu16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned short x[8];
    __m128i m;
  } src, src2, dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
    dst.x[i] = __SATURATE(src.x[i] - src2.x[i], 0, 65535);
  return dst.m;
#else
  return (__m128i)__builtin_ia32_psubusw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_and_si128(__m128i __a, __m128i __b)
{
  return __a & __b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_andnot_si128(__m128i __a, __m128i __b)
{
  return ~__a & __b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_or_si128(__m128i __a, __m128i __b)
{
  return __a | __b;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_xor_si128(__m128i __a, __m128i __b)
{
  return __a ^ __b;
}

#define _mm_slli_si128(a, imm) __extension__ ({                         \
  (__m128i)__builtin_shufflevector((__v16qi)_mm_setzero_si128(),        \
                                   (__v16qi)(__m128i)(a),               \
                                   ((imm)&0xF0) ? 0 : 16 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 17 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 18 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 19 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 20 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 21 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 22 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 23 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 24 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 25 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 26 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 27 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 28 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 29 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 30 - ((imm)&0xF), \
                                   ((imm)&0xF0) ? 0 : 31 - ((imm)&0xF)); })
#define _mm_bslli_si128(a, imm) \
  _mm_slli_si128((a), (imm))

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi16(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  return ((unsigned int)__count < 16) ? emscripten_int16x8_shiftLeftByScalar(__a, __count) : ((int16x8){ 0, 0, 0, 0, 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_psllwi128((__v8hi)__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi16(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
    return (__count[1] == 0 && (unsigned int)__count[0] < 16) ? emscripten_int16x8_shiftLeftByScalar(__a, __count[0]) : ((int16x8){ 0, 0, 0, 0, 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_psllw128((__v8hi)__a, (__v8hi)__count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi32(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  return ((unsigned int)__count < 32) ? emscripten_int32x4_shiftLeftByScalar(__a, __count) : ((int32x4){ 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_pslldi128((__v4si)__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi32(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
    return (__count[1] == 0 && (unsigned int)__count[0] < 32) ? emscripten_int32x4_shiftLeftByScalar(__a, __count[0]) : ((int32x4){ 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_pslld128((__v4si)__a, (__v4si)__count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_slli_epi64(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  if ((unsigned int)__count <= 63)
  {
    union {
      unsigned long long x[2];
      __m128i m;
    } __m;
    __m.m = __a;
    unsigned char __c = (unsigned char)__count;
    __m.x[0] = __m.x[0] << __c;
    __m.x[1] = __m.x[1] << __c;
    return __m.m;
  }
  else
    return (__m128i) { 0, 0, 0, 0 };
#else
  return __builtin_ia32_psllqi128(__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sll_epi64(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned long long x[2];
    __m128i m;
  } __m, __c;
  __c.m = __count;
  if (__c.x[0] <= 63)
  {
    __m.m = __a;
    __m.x[0] = __m.x[0] << __c.x[0];
    __m.x[1] = __m.x[1] << __c.x[0];
    return __m.m;
  }
  else
    return (__m128i) { 0, 0, 0, 0 };
#else
  return __builtin_ia32_psllq128(__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srai_epi16(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  return emscripten_int16x8_shiftRightByScalar(__a, (unsigned int)__count < 15 ? __count : 15);
#else
  return (__m128i)__builtin_ia32_psrawi128((__v8hi)__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sra_epi16(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
  return emscripten_int16x8_shiftRightByScalar(__a, (__count[1] == 0 && (unsigned int)__count[0] < 15) ? __count[0] : 15);
#else
  return (__m128i)__builtin_ia32_psraw128((__v8hi)__a, (__v8hi)__count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srai_epi32(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  return emscripten_int32x4_shiftRightByScalar(__a, (unsigned int)__count < 31 ? __count : 31);
#else
  return (__m128i)__builtin_ia32_psradi128((__v4si)__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sra_epi32(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
  return emscripten_int32x4_shiftRightByScalar(__a, (__count[1] == 0 && (unsigned int)__count[0] < 31) ? __count[0] : 31);
#else
  return (__m128i)__builtin_ia32_psrad128((__v4si)__a, (__v4si)__count);
#endif
}

#define _mm_srli_si128(a, imm) __extension__ ({                          \
  (__m128i)__builtin_shufflevector((__v16qi)(__m128i)(a),                \
                                   (__v16qi)_mm_setzero_si128(),         \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 0,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 1,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 2,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 3,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 4,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 5,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 6,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 7,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 8,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 9,  \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 10, \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 11, \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 12, \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 13, \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 14, \
                                   ((imm)&0xF0) ? 16 : ((imm)&0xF) + 15); })

#define _mm_bsrli_si128(a, imm) \
  _mm_srli_si128((a), (imm))

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi16(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  return ((unsigned int)__count < 16) ? ((int16x8)emscripten_uint16x8_shiftRightByScalar((uint16x8)__a, __count)) : ((int16x8){ 0, 0, 0, 0, 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_psrlwi128((__v8hi)__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi16(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
  return (__count[1] == 0 && (unsigned int)__count[0] < 32) ? ((int16x8)emscripten_uint16x8_shiftRightByScalar((uint16x8)__a, __count[0])) : ((int16x8){ 0, 0, 0, 0, 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_psrlw128((__v8hi)__a, (__v8hi)__count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi32(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  return ((unsigned int)__count < 32) ? ((int32x4)emscripten_uint32x4_shiftRightByScalar((uint32x4)__a, __count)) : ((int32x4){ 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_psrldi128((__v4si)__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi32(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
  return (__count[1] == 0 && (unsigned int)__count[0] < 32) ? ((int32x4)emscripten_uint32x4_shiftRightByScalar((uint32x4)__a, __count[0])) : ((int32x4){ 0, 0, 0, 0 });
#else
  return (__m128i)__builtin_ia32_psrld128((__v4si)__a, (__v4si)__count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srli_epi64(__m128i __a, int __count)
{
#ifdef __EMSCRIPTEN__
  if ((unsigned int)__count <= 63)
  {
    union {
      unsigned long long x[2];
      __m128i m;
    } __m;
    __m.m = __a;
    unsigned char __c = (unsigned char)__count;
    __m.x[0] = __m.x[0] >> __c;
    __m.x[1] = __m.x[1] >> __c;
    return __m.m;
  }
  else
    return (__m128i) { 0, 0, 0, 0 };
#else
  return __builtin_ia32_psrlqi128(__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_srl_epi64(__m128i __a, __m128i __count)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned long long x[2];
    __m128i m;
  } __m, __c;
  __c.m = __count;
  if (__c.x[0] <= 63)
  {
    __m.m = __a;
    __m.x[0] = __m.x[0] >> __c.x[0];
    __m.x[1] = __m.x[1] >> __c.x[0];
    return __m.m;
  }
  else
    return (__m128i) { 0, 0, 0, 0 };
#else
  return __builtin_ia32_psrlq128(__a, __count);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)((__v16qi)__a == (__v16qi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)((__v8hi)__a == (__v8hi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)((__v4si)__a == (__v4si)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)((__v16qs)__a > (__v16qs)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)((__v8hi)__a > (__v8hi)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)((__v4si)__a > (__v4si)__b);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi8(__m128i __a, __m128i __b)
{
  return _mm_cmpgt_epi8(__b, __a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi16(__m128i __a, __m128i __b)
{
  return _mm_cmpgt_epi16(__b, __a);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_epi32(__m128i __a, __m128i __b)
{
  return _mm_cmpgt_epi32(__b, __a);
}

#if defined(__x86_64__)
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_sd(__m128d __a, long long __b)
{
  __a[0] = __b;
  return __a;
}
#elif defined(__EMSCRIPTEN__)
static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_sd(__m128d __a, long long __b)
{
  union {
    double x[2];
    __m128d m;
  } m;
  m.m = __a;
  m.x[0] = (double)__b;
  return m.m;
}
#endif

#if defined(__x86_64__)
static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si64(__m128d __a)
{
  return __builtin_ia32_cvtsd2si64(__a);
}
#elif defined(__EMSCRIPTEN__)
static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsd_si64(__m128d __a)
{
  if (isnan(__a[0]) || isinf(__a[0])) return 0x8000000000000000LL;
  long long x = llrint(__a[0]);
  if (x != 0xFFFFFFFF00000000ULL && (x != 0 || fabsf(__a[0]) < 2.f))
    return x;
  else
    return 0x8000000000000000LL;
}
#endif

#if defined(__x86_64__)
static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si64(__m128d __a)
{
  return __a[0];
}
#elif defined(__EMSCRIPTEN__)
static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvttsd_si64(__m128d __a)
{
  if (isnan(__a[0]) || isinf(__a[0])) return 0x8000000000000000LL;
  long long x = llrint(__a[0]);
  if (x != 0xFFFFFFFF00000000ULL && (x != 0 || fabsf(__a[0]) < 2.f))
    return (long long)__a[0];
  else
    return 0x8000000000000000LL;
}
#endif

static __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtepi32_ps(__m128i __a)
{
#ifdef __EMSCRIPTEN__
  return emscripten_float32x4_fromInt32x4(__a);
#else
  return __builtin_ia32_cvtdq2ps((__v4si)__a);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtps_epi32(__m128 __a)
{
#ifdef __EMSCRIPTEN__
  // Emscripten: RangeError: SIMD conversion loses precision
  //return emscripten_int32x4_fromFloat32x4(__a);
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
#else
  return (__m128i)__builtin_ia32_cvtps2dq(__a);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvttps_epi32(__m128 __a)
{
#ifdef __EMSCRIPTEN__
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
#else
  return (__m128i)__builtin_ia32_cvttps2dq(__a);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_si128(int __a)
{
  return (__m128i)(__v4si){ __a, 0, 0, 0 };
}

#if defined(__x86_64__)
static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_si128(long long __a)
{
  return (__m128i){ __a, 0 };
}
#elif defined(__EMSCRIPTEN__)
static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_si128(long long __a)
{
  union {
    long long x[2];
    __m128i m;
  } m;
  m.x[0] = __a;
  m.x[1] = 0;
  return m.m;
}
#endif

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si32(__m128i __a)
{
  __v4si __b = (__v4si)__a;
  return __b[0];
}

#if defined(__x86_64__)
static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si64(__m128i __a)
{
  return __a[0];
}
#elif defined(__EMSCRIPTEN__)
static __inline__ long long __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi128_si64(__m128i __a)
{
  union {
    long long x[2];
    __m128i m;
  } m;
  m.m = __a;
  return m.x[0];
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_load_si128(__m128i const *__p)
{
  return *__p;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadu_si128(__m128i const *__p)
{
  struct __loadu_si128 {
    __m128i __v;
  } __attribute__((__packed__, __may_alias__));
  return ((struct __loadu_si128*)__p)->__v;
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_loadl_epi64(__m128i const *__p)
{
#ifdef __EMSCRIPTEN__
  struct __mm_loadl_epi64_struct {
    int __u[2];
  } __attribute__((__packed__, __may_alias__));
  return (__m128i) { ((struct __mm_loadl_epi64_struct*)__p)->__u[0], ((struct __mm_loadl_epi64_struct*)__p)->__u[1], 0, 0};
#else
  struct __mm_loadl_epi64_struct {
    long long __u;
  } __attribute__((__packed__, __may_alias__));
  return (__m128i) { ((struct __mm_loadl_epi64_struct*)__p)->__u, 0};
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi64x(long long q1, long long q0)
{
  return (__m128i){ q0, q1 };
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi64(__m64 q1, __m64 q0)
{
  return (__m128i){ (long long)q0, (long long)q1 };
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi32(int i3, int i2, int i1, int i0)
{
  return (__m128i)(__v4si){ i0, i1, i2, i3};
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi16(short w7, short w6, short w5, short w4, short w3, short w2, short w1, short w0)
{
  return (__m128i)(__v8hi){ w0, w1, w2, w3, w4, w5, w6, w7 };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set_epi8(char b15, char b14, char b13, char b12, char b11, char b10, char b9, char b8, char b7, char b6, char b5, char b4, char b3, char b2, char b1, char b0)
{
  return (__m128i)(__v16qi){ b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15 };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi64x(long long __q)
{
  return (__m128i){ __q, __q };
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi64(__m64 __q)
{
  return (__m128i){ (long long)__q, (long long)__q };
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi32(int __i)
{
  return (__m128i)(__v4si){ __i, __i, __i, __i };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi16(short __w)
{
  return (__m128i)(__v8hi){ __w, __w, __w, __w, __w, __w, __w, __w };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_set1_epi8(char __b)
{
  return (__m128i)(__v16qi){ __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b, __b };
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi64(__m64 q0, __m64 q1)
{
  return (__m128i){ (long long)q0, (long long)q1 };
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi32(int i0, int i1, int i2, int i3)
{
  return (__m128i)(__v4si){ i0, i1, i2, i3};
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi16(short w0, short w1, short w2, short w3, short w4, short w5, short w6, short w7)
{
  return (__m128i)(__v8hi){ w0, w1, w2, w3, w4, w5, w6, w7 };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setr_epi8(char b0, char b1, char b2, char b3, char b4, char b5, char b6, char b7, char b8, char b9, char b10, char b11, char b12, char b13, char b14, char b15)
{
  return (__m128i)(__v16qi){ b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15 };
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_setzero_si128(void)
{
  return (__m128i){ 0LL, 0LL };
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_si128(__m128i *__p, __m128i __b)
{
  *__p = __b;
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_si128(__m128i *__p, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  struct __unaligned {
    __m128i __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __unaligned *)__p)->__v = __b;
#else
  __builtin_ia32_storedqu((char *)__p, (__v16qi)__b);
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_maskmoveu_si128(__m128i __d, __m128i __n, char *__p)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } mask, data;
  mask.m = __n;
  data.m = __d;
  for(int i = 0; i < 16; ++i)
    if (mask.x[i] & 0x80)
      __p[i] = data.x[i];
#else
  __builtin_ia32_maskmovdqu((__v16qi)__d, (__v16qi)__n, __p);
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storel_epi64(__m128i *__p, __m128i __a)
{
#ifdef __EMSCRIPTEN__
  struct __mm_storel_epi64_struct {
    int __u[2];
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_storel_epi64_struct*)__p)->__u[0] = __a[0];
  ((struct __mm_storel_epi64_struct*)__p)->__u[1] = __a[1];
#else
  struct __mm_storel_epi64_struct {
    long long __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_storel_epi64_struct*)__p)->__u = __a[0];
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_pd(double *__p, __m128d __a)
{
#ifdef __EMSCRIPTEN__
  // Emscripten/SIMD.js does not have cache hinting.
  _mm_store_pd(__p, __a);
#else
  __builtin_ia32_movntpd(__p, __a);
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si128(__m128i *__p, __m128i __a)
{
#ifdef __EMSCRIPTEN__
  // Emscripten/SIMD.js does not have cache hinting.
  _mm_store_si128(__p, __a);
#else
  __builtin_ia32_movntdq(__p, __a);
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si32(int *__p, int __a)
{
#ifdef __EMSCRIPTEN__
  // No cache hinting available.
  /* TODO: Add a build flag EMSCRIPTEN_SIMD_REQUIRE_ELEMENT_ALIGNMENT or something similar to avoid this.
    Then could just do *__p = __a; */
  struct __unaligned {
    int __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __unaligned *)__p)->__v = __a;
#else
  __builtin_ia32_movnti(__p, __a);
#endif
}

#ifdef __x86_64__
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si64(long long *__p, long long __a)
{
  __builtin_ia32_movnti64(__p, __a);
}
#elif defined(__EMSCRIPTEN__)
typedef long long __attribute__((aligned(1))) align1_int64;
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_si64(long long *__p, long long __a)
{
  // No cache hinting available.
  emscripten_align1_int *__q = (emscripten_align1_int*)__p;
  memcpy(__q, &__a, sizeof(long long));
}
#endif

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_clflush(void const *__p)
{
#ifndef __EMSCRIPTEN__ // Emscripten/SIMD.js does not have cache hinting
  __builtin_ia32_clflush(__p);
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_lfence(void)
{
#ifdef __EMSCRIPTEN__
  __sync_synchronize(); // Emscripten/SharedArrayBuffer has only a full barrier instruction, which gives a stronger guarantee.
#else
  __builtin_ia32_lfence();
#endif
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_mfence(void)
{
#ifdef __EMSCRIPTEN__
  __sync_synchronize(); // Emscripten/SharedArrayBuffer has only a full barrier instruction, which gives a stronger guarantee.
#else
  __builtin_ia32_mfence();
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packs_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2;
  union {
    signed char x[16];
    __m128i m;
  } dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
  {
    dst.x[i] = __SATURATE(src.x[i], -128, 127);
    dst.x[8+i] = __SATURATE(src2.x[i], -128, 127);
  }
  return dst.m;
#else
  return (__m128i)__builtin_ia32_packsswb128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packs_epi32(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed int x[4];
    __m128i m;
  } src, src2;
  union {
    signed short x[8];
    __m128i m;
  } dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 4; ++i)
  {
    dst.x[i] = __SATURATE(src.x[i], -32768, 32767);
    dst.x[4+i] = __SATURATE(src2.x[i], -32768, 32767);
  }
  return dst.m;
#else
  return (__m128i)__builtin_ia32_packssdw128((__v4si)__a, (__v4si)__b);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_packus_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  union {
    signed short x[8];
    __m128i m;
  } src, src2;
  union {
    unsigned char x[16];
    __m128i m;
  } dst;
  src.m = __a;
  src2.m = __b;
  for(int i = 0; i < 8; ++i)
  {
    dst.x[i] = __SATURATE(src.x[i], 0, 255);
    dst.x[8+i] = __SATURATE(src2.x[i], 0, 255);
  }
  return dst.m;
#else
  return (__m128i)__builtin_ia32_packuswb128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_extract_epi16(__m128i __a, int __imm)
{
  __v8hi __b = (__v8hi)__a;
  return (unsigned short)__b[__imm & 7];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_insert_epi16(__m128i __a, int __b, int __imm)
{
  __v8hi __c = (__v8hi)__a;
  __c[__imm & 7] = __b;
  return (__m128i)__c;
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_epi8(__m128i __a)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned char x[16];
    __m128i m;
  } src;
  src.m = __a;
  unsigned int x = 0;
  for(int i = 0; i < 16; ++i)
    x |= ((unsigned int)src.x[i] >> 7) << i;
  return (int)x;
#else
  return __builtin_ia32_pmovmskb128((__v16qi)__a);
#endif
}

#define _mm_shuffle_epi32(a, imm) __extension__ ({ \
  (__m128i)__builtin_shufflevector((__v4si)(__m128i)(a), \
                                   (__v4si)_mm_set1_epi32(0), \
                                   (imm) & 0x3, ((imm) & 0xc) >> 2, \
                                   ((imm) & 0x30) >> 4, ((imm) & 0xc0) >> 6); })

#define _mm_shufflelo_epi16(a, imm) __extension__ ({ \
  (__m128i)__builtin_shufflevector((__v8hi)(__m128i)(a), \
                                   (__v8hi)_mm_set1_epi16(0), \
                                   (imm) & 0x3, ((imm) & 0xc) >> 2, \
                                   ((imm) & 0x30) >> 4, ((imm) & 0xc0) >> 6, \
                                   4, 5, 6, 7); })

#define _mm_shufflehi_epi16(a, imm) __extension__ ({ \
  (__m128i)__builtin_shufflevector((__v8hi)(__m128i)(a), \
                                   (__v8hi)_mm_set1_epi16(0), \
                                   0, 1, 2, 3, \
                                   4 + (((imm) & 0x03) >> 0), \
                                   4 + (((imm) & 0x0c) >> 2), \
                                   4 + (((imm) & 0x30) >> 4), \
                                   4 + (((imm) & 0xc0) >> 6)); })

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_shufflevector((__v16qi)__a, (__v16qi)__b, 8, 16+8, 9, 16+9, 10, 16+10, 11, 16+11, 12, 16+12, 13, 16+13, 14, 16+14, 15, 16+15);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_shufflevector((__v8hi)__a, (__v8hi)__b, 4, 8+4, 5, 8+5, 6, 8+6, 7, 8+7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_shufflevector((__v4si)__a, (__v4si)__b, 2, 4+2, 3, 4+3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_epi64(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  return (__m128i)__builtin_shufflevector(__a, __b, 2, 3, 4+2, 4+3);
#else
  return (__m128i)__builtin_shufflevector(__a, __b, 1, 2+1);
#endif
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi8(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_shufflevector((__v16qi)__a, (__v16qi)__b, 0, 16+0, 1, 16+1, 2, 16+2, 3, 16+3, 4, 16+4, 5, 16+5, 6, 16+6, 7, 16+7);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi16(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_shufflevector((__v8hi)__a, (__v8hi)__b, 0, 8+0, 1, 8+1, 2, 8+2, 3, 8+3);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_shufflevector((__v4si)__a, (__v4si)__b, 0, 4+0, 1, 4+1);
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_epi64(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
  return (__m128i)__builtin_shufflevector(__a, __b, 0, 1, 4+0, 4+1);
#else
  return (__m128i)__builtin_shufflevector(__a, __b, 0, 2+0);
#endif
}

#ifndef __EMSCRIPTEN__ // MMX support is not available in Emscripten/SIMD.js.
static __inline__ __m64 __attribute__((__always_inline__, __nodebug__))
_mm_movepi64_pi64(__m128i __a)
{
  return (__m64)__a[0];
}

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_movpi64_epi64(__m64 __a)
{
  return (__m128i){ (long long)__a, 0 };
}
#endif

static __inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_move_epi64(__m128i __a)
{
#ifdef __EMSCRIPTEN__
  return __builtin_shufflevector(__a, (__m128i){ 0 }, 0, 1, 4+0, 4+1);
#else
  return __builtin_shufflevector(__a, (__m128i){ 0 }, 0, 2);
#endif
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_pd(__m128d __a, __m128d __b)
{
  return __builtin_shufflevector(__a, __b, 1, 2+1);
}

static __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_pd(__m128d __a, __m128d __b)
{
  return __builtin_shufflevector(__a, __b, 0, 2+0);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_pd(__m128d __a)
{
#ifdef __EMSCRIPTEN__
  union {
    unsigned long long x[2];
    __m128d m;
  } src;
  src.m = __a;
  return (src.x[0] >> 63) | ((src.x[1] >> 63) << 1);
#else
  return __builtin_ia32_movmskpd(__a);
#endif
}

#define _mm_shuffle_pd(a, b, i) __extension__ ({ \
  __builtin_shufflevector((__m128d)(a), (__m128d)(b), \
                          (i) & 1, (((i) & 2) >> 1) + 2); })

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
#ifndef __EMSCRIPTEN__ // Emscripten does not have an equivalent.
  __asm__ volatile ("pause");
#endif
}

#define _MM_SHUFFLE2(x, y) (((x) << 1) | (y))

#endif /* __EMMINTRIN_H */
