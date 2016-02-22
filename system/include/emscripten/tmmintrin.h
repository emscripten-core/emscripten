/*===---- tmmintrin.h - SSSE3 intrinsics -----------------------------------===
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

#ifndef __TMMINTRIN_H
#define __TMMINTRIN_H

#include <pmmintrin.h>

#ifndef __SSSE3__
#error "SSSE3 instruction set not enabled"
#endif

/* Define the default attributes for the functions in this file. */
#ifdef __EMSCRIPTEN__
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))
#else
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("ssse3")))
#endif

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_abs_pi8(__m64 __a)
{
    return (__m64)__builtin_ia32_pabsb((__v8qi)__a);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_abs_epi8(__m128i __a)
{
#ifdef __EMSCRIPTEN__
    __m128i __mask = (__m128i)emscripten_int8x16_shiftRightByScalar((int8x16)__a, 7);
    return _mm_xor_si128(_mm_add_epi8(__a, __mask), __mask);
#else
    return (__m128i)__builtin_ia32_pabsb128((__v16qi)__a);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_abs_pi16(__m64 __a)
{
    return (__m64)__builtin_ia32_pabsw((__v4hi)__a);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_abs_epi16(__m128i __a)
{
#ifdef __EMSCRIPTEN__
    __m128i __mask = (__m128i)emscripten_int16x8_shiftRightByScalar((int16x8)__a, 15);
    return _mm_xor_si128(_mm_add_epi16(__a, __mask), __mask);
#else
    return (__m128i)__builtin_ia32_pabsw128((__v8hi)__a);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_abs_pi32(__m64 __a)
{
    return (__m64)__builtin_ia32_pabsd((__v2si)__a);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_abs_epi32(__m128i __a)
{
#ifdef __EMSCRIPTEN__
    __m128i __mask = (__m128i)emscripten_int32x4_shiftRightByScalar((int32x4)__a, 31);
    return _mm_xor_si128(_mm_add_epi32(__a, __mask), __mask);
#else
    return (__m128i)__builtin_ia32_pabsd128((__v4si)__a);
#endif
}

#ifdef __EMSCRIPTEN__
#define _mm_alignr_epi8(__a, __b, __count) \
    ((__count <= 16) \
    ? (_mm_or_si128(_mm_bslli_si128((__a), 16 - (((unsigned int)(__count)) & 0xFF)), _mm_bsrli_si128((__b), (((unsigned int)(__count)) & 0xFF)))) \
    : (_mm_bsrli_si128((__a), (((unsigned int)(__count)) & 0xFF) - 16)))
#else
#define _mm_alignr_epi8(a, b, n) __extension__ ({ \
  (__m128i)__builtin_ia32_palignr128((__v16qi)(__m128i)(a), \
                                     (__v16qi)(__m128i)(b), (n)); })
#endif

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
#define _mm_alignr_pi8(a, b, n) __extension__ ({ \
  (__m64)__builtin_ia32_palignr((__v8qi)(__m64)(a), (__v8qi)(__m64)(b), (n)); })
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_hadd_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    return _mm_add_epi16(__builtin_shufflevector((int16x8)__a, (int16x8)__b, 0, 2, 4, 6, 8, 10, 12, 14), __builtin_shufflevector((int16x8)__a, (int16x8)__b, 1, 3, 5, 7, 9, 11, 13, 15));
#else
    return (__m128i)__builtin_ia32_phaddw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_hadd_epi32(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    return _mm_add_epi32(__builtin_shufflevector(__a, __b, 0, 2, 4, 6), __builtin_shufflevector(__a, __b, 1, 3, 5, 7));
#else
    return (__m128i)__builtin_ia32_phaddd128((__v4si)__a, (__v4si)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_hadd_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_phaddw((__v4hi)__a, (__v4hi)__b);
}

static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_hadd_pi32(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_phaddd((__v2si)__a, (__v2si)__b);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_hadds_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    return (__m128i)emscripten_int16x8_addSaturate(__builtin_shufflevector((int16x8)__a, (int16x8)__b, 0, 2, 4, 6, 8, 10, 12, 14), __builtin_shufflevector((int16x8)__a, (int16x8)__b, 1, 3, 5, 7, 9, 11, 13, 15));
#else
    return (__m128i)__builtin_ia32_phaddsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_hadds_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_phaddsw((__v4hi)__a, (__v4hi)__b);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_hsub_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
     return _mm_sub_epi16(__builtin_shufflevector((int16x8)__a, (int16x8)__b, 0, 2, 4, 6, 8, 10, 12, 14), __builtin_shufflevector((int16x8)__a, (int16x8)__b, 1, 3, 5, 7, 9, 11, 13, 15));
#else
    return (__m128i)__builtin_ia32_phsubw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_hsub_epi32(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    return _mm_sub_epi32(__builtin_shufflevector(__a, __b, 0, 2, 4, 6), __builtin_shufflevector(__a, __b, 1, 3, 5, 7));
#else
    return (__m128i)__builtin_ia32_phsubd128((__v4si)__a, (__v4si)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_hsub_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_phsubw((__v4hi)__a, (__v4hi)__b);
}

static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_hsub_pi32(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_phsubd((__v2si)__a, (__v2si)__b);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_hsubs_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
     return (__m128i)emscripten_int16x8_subSaturate(__builtin_shufflevector((int16x8)__a, (int16x8)__b, 0, 2, 4, 6, 8, 10, 12, 14), __builtin_shufflevector((int16x8)__a, (int16x8)__b, 1, 3, 5, 7, 9, 11, 13, 15));
#else
    return (__m128i)__builtin_ia32_phsubsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_hsubs_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_phsubsw((__v4hi)__a, (__v4hi)__b);
}
#endif

#ifdef __EMSCRIPTEN__
static __inline__ short __DEFAULT_FN_ATTRS
__Saturate_To_Int16(int __x)
{
    return __x <= -32768 ? -32768 : (__x >= 32767 ? 32767 : __x);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_maddubs_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    union {
      char __x[16];
      short __s[8];
      __m128i __m;
    } __src, __src2, __dst;
    __src.__m = __a;
    __src2.__m = __b;
    for(int __i = 0; __i < 16; __i += 2)
        __dst.__s[__i>>1] = __Saturate_To_Int16((unsigned char)__src.__x[__i+1] * __src2.__x[__i+1] + (unsigned char)__src.__x[__i] * __src2.__x[__i]);
    return __dst.__m;
#else
    return (__m128i)__builtin_ia32_pmaddubsw128((__v16qi)__a, (__v16qi)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_maddubs_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_pmaddubsw((__v8qi)__a, (__v8qi)__b);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_mulhrs_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    union {
      short __x[8];
      __m128i __m;
    } __src, __src2, __dst;
    __src.__m = __a;
    __src2.__m = __b;
    for(int __i = 0; __i < 8; ++__i)
        __dst.__x[__i] = (((__src.__x[__i] * __src2.__x[__i]) >> 14) + 1) >> 1;
    return __dst.__m;
#else
    return (__m128i)__builtin_ia32_pmulhrsw128((__v8hi)__a, (__v8hi)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_mulhrs_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_pmulhrsw((__v4hi)__a, (__v4hi)__b);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_shuffle_epi8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    union {
      unsigned char __x[16];
      __m128i __m;
    } __src, __src2, __dst;
    __src.__m = __a;
    __src2.__m = __b;
    for(int __i = 0; __i < 16; ++__i)
        __dst.__x[__i] = (__src2.__x[__i] & 0x80) ? 0 : __src.__x[__src2.__x[__i]&15];
    return __dst.__m;
#else
    return (__m128i)__builtin_ia32_pshufb128((__v16qi)__a, (__v16qi)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_shuffle_pi8(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_pshufb((__v8qi)__a, (__v8qi)__b);
}
#endif

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_sign_epi8(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    __m128i __mask = (__m128i)emscripten_int8x16_shiftRightByScalar((int8x16)__b, 7);
    __m128i __zeromask = (__m128i)emscripten_int8x16_notEqual((int8x16)__b, emscripten_int8x16_splat(0));
    return _mm_and_si128(__zeromask, _mm_xor_si128(_mm_add_epi8(__a, __mask), __mask));
#else
    return (__m128i)__builtin_ia32_psignb128((__v16qi)__a, (__v16qi)__b);
#endif
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_sign_epi16(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    __m128i __mask = (__m128i)emscripten_int16x8_shiftRightByScalar((int16x8)__b, 15);
    __m128i __zeromask = (__m128i)emscripten_int16x8_notEqual((int16x8)__b, emscripten_int16x8_splat(0));
    return _mm_and_si128(__zeromask, _mm_xor_si128(_mm_add_epi16(__a, __mask), __mask));
#else
    return (__m128i)__builtin_ia32_psignw128((__v8hi)__a, (__v8hi)__b);
#endif
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_sign_epi32(__m128i __a, __m128i __b)
{
#ifdef __EMSCRIPTEN__
    __m128i __mask = (__m128i)emscripten_int32x4_shiftRightByScalar((int32x4)__b, 31);
    __m128i __zeromask = (__m128i)emscripten_int32x4_notEqual((int32x4)__b, emscripten_int32x4_splat(0));
    return _mm_and_si128(__zeromask, _mm_xor_si128(_mm_add_epi32(__a, __mask), __mask));
#else
    return (__m128i)__builtin_ia32_psignd128((__v4si)__a, (__v4si)__b);
#endif
}

#ifndef __EMSCRIPTEN__ /* MMX registers/__m64 type is not available in Emscripten. */
static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_sign_pi8(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_psignb((__v8qi)__a, (__v8qi)__b);
}

static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_sign_pi16(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_psignw((__v4hi)__a, (__v4hi)__b);
}

static __inline__ __m64 __DEFAULT_FN_ATTRS
_mm_sign_pi32(__m64 __a, __m64 __b)
{
    return (__m64)__builtin_ia32_psignd((__v2si)__a, (__v2si)__b);
}
#endif

#undef __DEFAULT_FN_ATTRS

#endif /* __TMMINTRIN_H */
