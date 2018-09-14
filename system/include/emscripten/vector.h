/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __emscripten_vector_h__
#define __emscripten_vector_h__

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__cplusplus)
#define EM_INLINE inline
#else
// Compiling with -std=c89 or -std=c90 which did not have inline, compile all inline
// definitions as static.
#define EM_INLINE static
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Direct mappings to the SIMD.js API specification as intrinsic functions: http://tc39.github.io/ecmascript_simd/
// These can be used as an alternative to using SSEx intrinsics and LLVM/GCC built-in intrinsics support.

// Note: Since there are a lot of types and functions, when modifying this file, please keep a canonical sorted ordering of types
// float64x2->float32x4->int64x2->uint64x2->int32x4->uint32x4->int16x8->uint16x8->int8x16->uint8x16->bool64x2->bool32x4->bool16x8->bool8x16
// and all functions in the list order specified in the EcmaScript SIMD.js specification
// so that cross-referencing changes against the implementation and the specification is much easier.

// Also, if changing this file, remember to do the matching changes in LLVM side at https://github.com/kripken/emscripten-fastcomp/blob/incoming/lib/Target/JSBackend/CallHandlers.h

typedef double float64x2 __attribute__((__vector_size__(16), __may_alias__));
typedef float float32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef long long int64x2 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned long long uint64x2 __attribute__((__vector_size__(16), __may_alias__));
typedef int int32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned int uint32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef short int16x8 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned short uint16x8 __attribute__((__vector_size__(16), __may_alias__));
typedef char int8x16 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned char uint8x16 __attribute__((__vector_size__(16), __may_alias__));

// C/C++ side code does not actually have the boolean types, these are marked for documentation purposes. Feel free
// to use the int SIMD types in user code as well.
typedef int32x4 bool64x2; // int64x2 would give trouble, since that type doesn't exist and we aren't prepared to handle it in LLVM side.
typedef int32x4 bool32x4;
typedef int16x8 bool16x8;
typedef int8x16 bool8x16;

// Float64x2
float64x2 emscripten_float64x2_set(double __s0, double __s1) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_splat(double __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_float64x2_check, not relevant for C/C++.
EM_INLINE float64x2 emscripten_float64x2_add(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE float64x2 emscripten_float64x2_sub(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE float64x2 emscripten_float64x2_mul(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
EM_INLINE float64x2 emscripten_float64x2_div(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a / __b; }
float64x2 emscripten_float64x2_max(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_min(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_maxNum(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_minNum(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
EM_INLINE float64x2 emscripten_float64x2_neg(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
float64x2 emscripten_float64x2_sqrt(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_reciprocalApproximation(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_reciprocalSqrtApproximation(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_abs(float64x2 __a) __attribute__((__nothrow__, __const__));
#define emscripten_float64x2_and(__a, __b) emscripten_float64x2_fromInt32x4Bits(emscripten_int32x4_and(emscripten_int32x4_fromFloat64x2Bits((__a)), emscripten_int32x4_fromFloat64x2Bits((__b))))
#define emscripten_float64x2_xor(__a, __b) emscripten_float64x2_fromInt32x4Bits(emscripten_int32x4_xor(emscripten_int32x4_fromFloat64x2Bits((__a)), emscripten_int32x4_fromFloat64x2Bits((__b))))
#define emscripten_float64x2_or(__a, __b) emscripten_float64x2_fromInt32x4Bits(emscripten_int32x4_or(emscripten_int32x4_fromFloat64x2Bits((__a)), emscripten_int32x4_fromFloat64x2Bits((__b))))
#define emscripten_float64x2_not(__a) emscripten_float64x2_fromInt32x4Bits(emscripten_int32x4_not(emscripten_int32x4_fromFloat64x2Bits((__a))))
bool64x2 emscripten_float64x2_lessThan(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
bool64x2 emscripten_float64x2_lessThanOrEqual(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
bool64x2 emscripten_float64x2_greaterThan(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
bool64x2 emscripten_float64x2_greaterThanOrEqual(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
bool64x2 emscripten_float64x2_equal(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
bool64x2 emscripten_float64x2_notEqual(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_float64x2_anyTrue, only defined on boolean SIMD types.
// n.b. No emscripten_float64x2_allTrue, only defined on boolean SIMD types.
float64x2 emscripten_float64x2_select(bool64x2 __a, float64x2 __b, float64x2 __c) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_float64x2_addSaturate, only defined on 8-bit and 16-bit integer SIMD types.
// n.b. No emscripten_float64x2_subSaturate, only defined on 8-bit and 16-bit integer SIMD types.
// n.b. No emscripten_float64x2_shiftLeftByScalar, only defined on integer SIMD types.
// n.b. No emscripten_float64x2_shiftRightByScalar, only defined on integer SIMD types.
EM_INLINE double emscripten_float64x2_extractLane(float64x2 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE float64x2 emscripten_float64x2_replaceLane(float64x2 __a, int __lane, double __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_float64x2_store(const void *__p, float64x2 __a) __attribute__((__nothrow__));
void emscripten_float64x2_store1(const void *__p, float64x2 __a) __attribute__((__nothrow__));
float64x2 emscripten_float64x2_load(const void *__p) __attribute__((__nothrow__, __pure__));
float64x2 emscripten_float64x2_load1(const void *__p) __attribute__((__nothrow__, __pure__));
float64x2 emscripten_float64x2_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__));
EM_INLINE float64x2 emscripten_float64x2_fromInt32x4Bits(int32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float64x2)__a; }
EM_INLINE float64x2 emscripten_float64x2_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float64x2)__a; }
EM_INLINE float64x2 emscripten_float64x2_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float64x2)__a; }
EM_INLINE float64x2 emscripten_float64x2_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float64x2)__a; }
EM_INLINE float64x2 emscripten_float64x2_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float64x2)__a; }
EM_INLINE float64x2 emscripten_float64x2_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float64x2)__a; }
float64x2 emscripten_float64x2_swizzle(float64x2 __a, int __lane0, int __lane1) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_shuffle(float64x2 __a, float64x2 __b, int __lane0, int __lane1) __attribute__((__nothrow__, __const__));

// Float32x4
float32x4 emscripten_float32x4_set(float __s0, float __s1, float __s2, float __s3) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_splat(float __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_float32x4_check, not relevant for C/C++.
EM_INLINE float32x4 emscripten_float32x4_add(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE float32x4 emscripten_float32x4_sub(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE float32x4 emscripten_float32x4_mul(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
EM_INLINE float32x4 emscripten_float32x4_div(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a / __b; }
float32x4 emscripten_float32x4_max(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_min(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_maxNum(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_minNum(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
EM_INLINE float32x4 emscripten_float32x4_neg(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
float32x4 emscripten_float32x4_sqrt(float32x4 __a) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_reciprocalApproximation(float32x4 __a) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_reciprocalSqrtApproximation(float32x4 __a) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_abs(float32x4 __a) __attribute__((__nothrow__, __const__));
#define emscripten_float32x4_and(__a, __b) emscripten_float32x4_fromInt32x4Bits(emscripten_int32x4_and(emscripten_int32x4_fromFloat32x4Bits((__a)), emscripten_int32x4_fromFloat32x4Bits((__b))))
#define emscripten_float32x4_xor(__a, __b) emscripten_float32x4_fromInt32x4Bits(emscripten_int32x4_xor(emscripten_int32x4_fromFloat32x4Bits((__a)), emscripten_int32x4_fromFloat32x4Bits((__b))))
#define emscripten_float32x4_or(__a, __b) emscripten_float32x4_fromInt32x4Bits(emscripten_int32x4_or(emscripten_int32x4_fromFloat32x4Bits((__a)), emscripten_int32x4_fromFloat32x4Bits((__b))))
#define emscripten_float32x4_not(__a) emscripten_float32x4_fromInt32x4Bits(emscripten_int32x4_not(emscripten_int32x4_fromFloat32x4Bits(__a)))
EM_INLINE bool32x4 emscripten_float32x4_lessThan(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool32x4 emscripten_float32x4_lessThanOrEqual(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool32x4 emscripten_float32x4_greaterThan(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool32x4 emscripten_float32x4_greaterThanOrEqual(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool32x4 emscripten_float32x4_equal(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool32x4 emscripten_float32x4_notEqual(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
// n.b. No emscripten_float32x4_anyTrue, only defined on boolean SIMD types.
// n.b. No emscripten_float32x4_allTrue, only defined on boolean SIMD types.
float32x4 emscripten_float32x4_select(bool32x4 __a, float32x4 __b, float32x4 __c) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_float32x4_addSaturate, only defined on 8-bit and 16-bit integer SIMD types.
// n.b. No emscripten_float32x4_subSaturate, only defined on 8-bit and 16-bit integer SIMD types.
// n.b. No emscripten_float32x4_shiftLeftByScalar, only defined on integer SIMD types.
// n.b. No emscripten_float32x4_shiftRightByScalar, only defined on integer SIMD types.
EM_INLINE float emscripten_float32x4_extractLane(float32x4 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE float32x4 emscripten_float32x4_replaceLane(float32x4 __a, int __lane, float __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_float32x4_store(const void *__p, float32x4 __a) __attribute__((__nothrow__));
void emscripten_float32x4_store1(const void *__p, float32x4 __a) __attribute__((__nothrow__));
void emscripten_float32x4_store2(const void *__p, float32x4 __a) __attribute__((__nothrow__));
float32x4 emscripten_float32x4_load(const void *__p) __attribute__((__nothrow__, __pure__));
float32x4 emscripten_float32x4_load1(const void *__p) __attribute__((__nothrow__, __pure__));
float32x4 emscripten_float32x4_load2(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE float32x4 emscripten_float32x4_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromInt32x4Bits(int32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (float32x4)__a; }
EM_INLINE float32x4 emscripten_float32x4_fromInt32x4(int32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, float32x4); }
EM_INLINE float32x4 emscripten_float32x4_fromUint32x4(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, float32x4); }
float32x4 emscripten_float32x4_swizzle(float32x4 __a, int __lane0, int __lane1, int __lane2, int __lane3) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_shuffle(float32x4 __a, float32x4 __b, int __lane0, int __lane1, int __lane2, int __lane3) __attribute__((__nothrow__, __const__));

// Int32x4
int32x4 emscripten_int32x4_set(int __s0, int __s1, int __s2, int __s3) __attribute__((__nothrow__, __const__));
int32x4 emscripten_int32x4_splat(int __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_int32x4_check, not relevant for C/C++.
EM_INLINE int32x4 emscripten_int32x4_add(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE int32x4 emscripten_int32x4_sub(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE int32x4 emscripten_int32x4_mul(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
// n.b. No emscripten_int32x4_div, division is only defined on floating point types.
// n.b. No emscripten_int32x4_max, only defined on floating point types.
// n.b. No emscripten_int32x4_min, only defined on floating point types.
// n.b. No emscripten_int32x4_maxNum, only defined on floating point types.
// n.b. No emscripten_int32x4_minNum, only defined on floating point types.
EM_INLINE int32x4 emscripten_int32x4_neg(int32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
// n.b. No emscripten_int32x4_sqrt, only defined on floating point types.
// n.b. No emscripten_int32x4_reciprocalApproximation, only defined on floating point types.
// n.b. No emscripten_int32x4_reciprocalSqrtApproximation, only defined on floating point types.
// n.b. No emscripten_int32x4_abs, only defined on floating point types.
EM_INLINE int32x4 emscripten_int32x4_and(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a & __b; }
EM_INLINE int32x4 emscripten_int32x4_xor(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a ^ __b; }
EM_INLINE int32x4 emscripten_int32x4_or(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a | __b; }
EM_INLINE int32x4 emscripten_int32x4_not(int32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return ~__a; }
EM_INLINE bool32x4 emscripten_int32x4_lessThan(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool32x4 emscripten_int32x4_lessThanOrEqual(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool32x4 emscripten_int32x4_greaterThan(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool32x4 emscripten_int32x4_greaterThanOrEqual(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool32x4 emscripten_int32x4_equal(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool32x4 emscripten_int32x4_notEqual(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
#define emscripten_int32x4_anyTrue emscripten_bool32x4_anyTrue
#define emscripten_int32x4_allTrue emscripten_bool32x4_allTrue
int32x4 emscripten_int32x4_select(int32x4 __a, int32x4 __b, int32x4 __c) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_int32x4_addSaturate, only defined on 8-bit and 16-bit integer SIMD types.
// n.b. No emscripten_int32x4_subSaturate, only defined on 8-bit and 16-bit integer SIMD types.
int32x4 emscripten_int32x4_shiftLeftByScalar(int32x4 __a, int __count) __attribute__((__nothrow__, __const__));
int32x4 emscripten_int32x4_shiftRightByScalar(int32x4 __a, int __count) __attribute__((__nothrow__, __const__)); // Arithmetic right shift, i.e. sign-extending, or integer division
EM_INLINE int emscripten_int32x4_extractLane(int32x4 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE int32x4 emscripten_int32x4_replaceLane(int32x4 __a, int __lane, int __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_int32x4_store(const void *__p, int32x4 __a) __attribute__((__nothrow__));
void emscripten_int32x4_store1(const void *__p, int32x4 __a) __attribute__((__nothrow__));
void emscripten_int32x4_store2(const void *__p, int32x4 __a) __attribute__((__nothrow__));
int32x4 emscripten_int32x4_load(const void *__p) __attribute__((__nothrow__, __pure__));
int32x4 emscripten_int32x4_load1(const void *__p) __attribute__((__nothrow__, __pure__));
int32x4 emscripten_int32x4_load2(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE int32x4 emscripten_int32x4_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int32x4)__a; }
EM_INLINE int32x4 emscripten_int32x4_fromFloat32x4(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, int32x4); }
EM_INLINE int32x4 emscripten_int32x4_fromUint32x4(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, int32x4); }
int32x4 emscripten_int32x4_fromFloat64x2(float64x2 __a) __attribute__((__nothrow__, __const__)); // Unofficial, converts the two Float64x2 to two lowest lanes of an Int32x4, and sets the higher two lanes to zero.
int32x4 emscripten_int32x4_swizzle(int32x4 __a, int __lane0, int __lane1, int __lane2, int __lane3) __attribute__((__nothrow__, __const__));
int32x4 emscripten_int32x4_shuffle(int32x4 __a, int32x4 __b, int __lane0, int __lane1, int __lane2, int __lane3) __attribute__((__nothrow__, __const__));

// Uint32x4
uint32x4 emscripten_uint32x4_set(unsigned int __s0, unsigned int __s1, unsigned int __s2, unsigned int __s3) __attribute__((__nothrow__, __const__));
uint32x4 emscripten_uint32x4_splat(unsigned int __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_uint32x4_check, not relevant for C/C++.
EM_INLINE uint32x4 emscripten_uint32x4_add(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE uint32x4 emscripten_uint32x4_sub(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE uint32x4 emscripten_uint32x4_mul(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
// n.b. No emscripten_uint32x4_div, division is only defined on floating point types.
// n.b. No emscripten_uint32x4_max, only defined on floating point types.
// n.b. No emscripten_uint32x4_min, only defined on floating point types.
// n.b. No emscripten_uint32x4_maxNum, only defined on floating point types.
// n.b. No emscripten_uint32x4_minNum, only defined on floating point types.
EM_INLINE uint32x4 emscripten_uint32x4_neg(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
// n.b. No emscripten_uint32x4_sqrt, only defined on floating point types.
// n.b. No emscripten_uint32x4_reciprocalApproximation, only defined on floating point types.
// n.b. No emscripten_uint32x4_reciprocalSqrtApproximation, only defined on floating point types.
// n.b. No emscripten_uint32x4_abs, only defined on floating point types.
EM_INLINE uint32x4 emscripten_uint32x4_and(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a & __b; }
EM_INLINE uint32x4 emscripten_uint32x4_xor(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a ^ __b; }
EM_INLINE uint32x4 emscripten_uint32x4_or(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a | __b; }
EM_INLINE uint32x4 emscripten_uint32x4_not(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return ~__a; }
EM_INLINE bool32x4 emscripten_uint32x4_lessThan(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool32x4 emscripten_uint32x4_lessThanOrEqual(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool32x4 emscripten_uint32x4_greaterThan(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool32x4 emscripten_uint32x4_greaterThanOrEqual(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool32x4 emscripten_uint32x4_equal(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool32x4 emscripten_uint32x4_notEqual(uint32x4 __a, uint32x4 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
#define emscripten_uint32x4_anyTrue emscripten_bool32x4_anyTrue
#define emscripten_uint32x4_allTrue emscripten_bool32x4_allTrue
uint32x4 emscripten_uint32x4_select(uint32x4 __a, uint32x4 __b, uint32x4 __c) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_uint32x4_addSaturate, only defined on 8-bit and 16-bit integer SIMD types.
// n.b. No emscripten_uint32x4_subSaturate, only defined on 8-bit and 16-bit integer SIMD types.
uint32x4 emscripten_uint32x4_shiftLeftByScalar(uint32x4 __a, int __count) __attribute__((__nothrow__, __const__));
uint32x4 emscripten_uint32x4_shiftRightByScalar(uint32x4 __a, int __count) __attribute__((__nothrow__, __const__));
EM_INLINE int emscripten_uint32x4_extractLane(uint32x4 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE uint32x4 emscripten_uint32x4_replaceLane(uint32x4 __a, int __lane, int __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_uint32x4_store(const void *__p, uint32x4 __a) __attribute__((__nothrow__));
void emscripten_uint32x4_store1(const void *__p, uint32x4 __a) __attribute__((__nothrow__));
void emscripten_uint32x4_store2(const void *__p, uint32x4 __a) __attribute__((__nothrow__));
uint32x4 emscripten_uint32x4_load(const void *__p) __attribute__((__nothrow__, __pure__));
uint32x4 emscripten_uint32x4_load1(const void *__p) __attribute__((__nothrow__, __pure__));
uint32x4 emscripten_uint32x4_load2(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE uint32x4 emscripten_uint32x4_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromInt32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint32x4)__a; }
EM_INLINE uint32x4 emscripten_uint32x4_fromFloat32x4(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, uint32x4); }
EM_INLINE uint32x4 emscripten_uint32x4_fromInt32x4(int32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, uint32x4); }
uint32x4 emscripten_uint32x4_fromFloat64x2(float64x2 __a) __attribute__((__nothrow__, __const__)); // Unofficial, converts the two Float64x2 to two lowest lanes of an Uint32x4, and sets the higher two lanes to zero.
uint32x4 emscripten_uint32x4_swizzle(uint32x4 __a, int __lane0, int __lane1, int __lane2, int __lane3) __attribute__((__nothrow__, __const__));
uint32x4 emscripten_uint32x4_shuffle(uint32x4 __a, uint32x4 __b, int __lane0, int __lane1, int __lane2, int __lane3) __attribute__((__nothrow__, __const__));

// Int16x8
int16x8 emscripten_int16x8_set(short __s0, short __s1, short __s2, short __s3, short __s4, short __s5, short __s6, short __s7) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_splat(short __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_int16x8_check, not relevant for C/C++.
EM_INLINE int16x8 emscripten_int16x8_add(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE int16x8 emscripten_int16x8_sub(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE int16x8 emscripten_int16x8_mul(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
// n.b. No emscripten_int16x8_div, division is only defined on floating point types.
// n.b. No emscripten_int16x8_max, only defined on floating point types.
// n.b. No emscripten_int16x8_min, only defined on floating point types.
// n.b. No emscripten_int16x8_maxNum, only defined on floating point types.
// n.b. No emscripten_int16x8_minNum, only defined on floating point types.
EM_INLINE int16x8 emscripten_int16x8_neg(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
// n.b. No emscripten_int16x8_sqrt, only defined on floating point types.
// n.b. No emscripten_int16x8_reciprocalApproximation, only defined on floating point types.
// n.b. No emscripten_int16x8_reciprocalSqrtApproximation, only defined on floating point types.
// n.b. No emscripten_int16x8_abs, only defined on floating point types.
EM_INLINE int16x8 emscripten_int16x8_and(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a & __b; }
EM_INLINE int16x8 emscripten_int16x8_xor(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a ^ __b; }
EM_INLINE int16x8 emscripten_int16x8_or(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a | __b; }
EM_INLINE int16x8 emscripten_int16x8_not(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return ~__a; }
EM_INLINE bool16x8 emscripten_int16x8_lessThan(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool16x8 emscripten_int16x8_lessThanOrEqual(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool16x8 emscripten_int16x8_greaterThan(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool16x8 emscripten_int16x8_greaterThanOrEqual(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool16x8 emscripten_int16x8_equal(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool16x8 emscripten_int16x8_notEqual(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
#define emscripten_int16x8_anyTrue emscripten_bool16x8_anyTrue
#define emscripten_int16x8_allTrue emscripten_bool16x8_allTrue
int16x8 emscripten_int16x8_select(bool16x8 __a, int16x8 __b, int16x8 __c) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_addSaturate(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_subSaturate(int16x8 __a, int16x8 __b) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_shiftLeftByScalar(int16x8 __a, int __count) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_shiftRightByScalar(int16x8 __a, int __count) __attribute__((__nothrow__, __const__));
EM_INLINE short emscripten_int16x8_extractLane(int16x8 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE int16x8 emscripten_int16x8_replaceLane(int16x8 __a, int __lane, short __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_int16x8_store(const void *__p, int16x8 __a) __attribute__((__nothrow__));
int16x8 emscripten_int16x8_load(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE int16x8 emscripten_int16x8_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromInt32x4Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int16x8)__a; }
EM_INLINE int16x8 emscripten_int16x8_fromUint16x8(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, int16x8); }
int16x8 emscripten_int16x8_swizzle(int16x8 __a, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_shuffle(int16x8 __a, int16x8 __b, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7) __attribute__((__nothrow__, __const__));

// Uint16x8
uint16x8 emscripten_uint16x8_set(unsigned short __s0, unsigned short __s1, unsigned short __s2, unsigned short __s3, unsigned short __s4, unsigned short __s5, unsigned short __s6, unsigned short __s7) __attribute__((__nothrow__, __const__));
uint16x8 emscripten_uint16x8_splat(unsigned short __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_uint16x8_check, not relevant for C/C++.
EM_INLINE uint16x8 emscripten_uint16x8_add(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE uint16x8 emscripten_uint16x8_sub(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE uint16x8 emscripten_uint16x8_mul(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
// n.b. No emscripten_uint16x8_div, division is only defined on floating point types.
// n.b. No emscripten_uint16x8_max, only defined on floating point types.
// n.b. No emscripten_uint16x8_min, only defined on floating point types.
// n.b. No emscripten_uint16x8_maxNum, only defined on floating point types.
// n.b. No emscripten_uint16x8_minNum, only defined on floating point types.
EM_INLINE uint16x8 emscripten_uint16x8_neg(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
// n.b. No emscripten_uint16x8_sqrt, only defined on floating point types.
// n.b. No emscripten_uint16x8_reciprocalApproximation, only defined on floating point types.
// n.b. No emscripten_uint16x8_reciprocalSqrtApproximation, only defined on floating point types.
// n.b. No emscripten_uint16x8_abs, only defined on floating point types.
EM_INLINE uint16x8 emscripten_uint16x8_and(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a & __b; }
EM_INLINE uint16x8 emscripten_uint16x8_xor(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a ^ __b; }
EM_INLINE uint16x8 emscripten_uint16x8_or(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a | __b; }
EM_INLINE uint16x8 emscripten_uint16x8_not(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return ~__a; }
EM_INLINE bool16x8 emscripten_uint16x8_lessThan(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool16x8 emscripten_uint16x8_lessThanOrEqual(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool16x8 emscripten_uint16x8_greaterThan(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool16x8 emscripten_uint16x8_greaterThanOrEqual(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool16x8 emscripten_uint16x8_equal(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool16x8 emscripten_uint16x8_notEqual(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
#define emscripten_uint16x8_anyTrue emscripten_bool16x8_anyTrue
#define emscripten_uint16x8_allTrue emscripten_bool16x8_allTrue
uint16x8 emscripten_uint16x8_select(bool16x8 __a, uint16x8 __b, uint16x8 __c) __attribute__((__nothrow__, __const__));
uint16x8 emscripten_uint16x8_addSaturate(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__));
uint16x8 emscripten_uint16x8_subSaturate(uint16x8 __a, uint16x8 __b) __attribute__((__nothrow__, __const__));
uint16x8 emscripten_uint16x8_shiftLeftByScalar(uint16x8 __a, int __count) __attribute__((__nothrow__, __const__));
uint16x8 emscripten_uint16x8_shiftRightByScalar(uint16x8 __a, int __count) __attribute__((__nothrow__, __const__));
EM_INLINE unsigned short emscripten_uint16x8_extractLane(uint16x8 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE uint16x8 emscripten_uint16x8_replaceLane(uint16x8 __a, int __lane, unsigned short __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_uint16x8_store(const void *__p, uint16x8 __a) __attribute__((__nothrow__));
uint16x8 emscripten_uint16x8_load(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE uint16x8 emscripten_uint16x8_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromInt32x4Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint16x8)__a; }
EM_INLINE uint16x8 emscripten_uint16x8_fromInt16x8(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, uint16x8); }
uint16x8 emscripten_uint16x8_swizzle(uint16x8 __a, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7) __attribute__((__nothrow__, __const__));
uint16x8 emscripten_uint16x8_shuffle(uint16x8 __a, uint16x8 __b, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7) __attribute__((__nothrow__, __const__));

// Int8x16
int8x16 emscripten_int8x16_set(char __s0, char __s1, char __s2, char __s3, char __s4, char __s5, char __s6, char __s7, char __s8, char __s9, char __s10, char __s11, char __s12, char __s13, char __s14, char __s15) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_splat(char __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_int8x16_check, not relevant for C/C++.
EM_INLINE int8x16 emscripten_int8x16_add(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE int8x16 emscripten_int8x16_sub(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE int8x16 emscripten_int8x16_mul(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
// n.b. No emscripten_int8x16_div, division is only defined on floating point types.
// n.b. No emscripten_int8x16_max, only defined on floating point types.
// n.b. No emscripten_int8x16_min, only defined on floating point types.
// n.b. No emscripten_int8x16_maxNum, only defined on floating point types.
// n.b. No emscripten_int8x16_minNum, only defined on floating point types.
EM_INLINE int8x16 emscripten_int8x16_neg(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
// n.b. No emscripten_int8x16_sqrt, only defined on floating point types.
// n.b. No emscripten_int8x16_reciprocalApproximation, only defined on floating point types.
// n.b. No emscripten_int8x16_reciprocalSqrtApproximation, only defined on floating point types.
// n.b. No emscripten_int8x16_abs, only defined on floating point types.
EM_INLINE int8x16 emscripten_int8x16_and(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a & __b; }
EM_INLINE int8x16 emscripten_int8x16_xor(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a ^ __b; }
EM_INLINE int8x16 emscripten_int8x16_or(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a | __b; }
EM_INLINE int8x16 emscripten_int8x16_not(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return ~__a; }
EM_INLINE bool8x16 emscripten_int8x16_lessThan(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool8x16 emscripten_int8x16_lessThanOrEqual(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool8x16 emscripten_int8x16_greaterThan(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool8x16 emscripten_int8x16_greaterThanOrEqual(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool8x16 emscripten_int8x16_equal(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool8x16 emscripten_int8x16_notEqual(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
#define emscripten_int8x16_anyTrue emscripten_bool8x16_anyTrue
#define emscripten_int8x16_allTrue emscripten_bool8x16_allTrue
int8x16 emscripten_int8x16_select(bool8x16 __a, int8x16 __b, int8x16 __c) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_addSaturate(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_subSaturate(int8x16 __a, int8x16 __b) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_shiftLeftByScalar(int8x16 __a, int __count) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_shiftRightByScalar(int8x16 __a, int __count) __attribute__((__nothrow__, __const__));
EM_INLINE char emscripten_int8x16_extractLane(int8x16 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE int8x16 emscripten_int8x16_replaceLane(int8x16 __a, int __lane, char __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_int8x16_store(const void *__p, int8x16 __a) __attribute__((__nothrow__));
int8x16 emscripten_int8x16_load(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE int8x16 emscripten_int8x16_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromInt32x4Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromUint8x16Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (int8x16)__a; }
EM_INLINE int8x16 emscripten_int8x16_fromUint8x16(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, int8x16); }
int8x16 emscripten_int8x16_swizzle(int8x16 __a, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7, int __lane8, int __lane9, int __lane10, int __lane11, int __lane12, int __lane13, int __lane14, int __lane15) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_shuffle(int8x16 __a, int8x16 __b, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7, int __lane8, int __lane9, int __lane10, int __lane11, int __lane12, int __lane13, int __lane14, int __lane15) __attribute__((__nothrow__, __const__));

// Uint8x16
int8x16 emscripten_uint8x16_set(unsigned char __s0, unsigned char __s1, unsigned char __s2, unsigned char __s3, unsigned char __s4, unsigned char __s5, unsigned char __s6, unsigned char __s7, unsigned char __s8, unsigned char __s9, unsigned char __s10, unsigned char __s11, unsigned char __s12, unsigned char __s13, unsigned char __s14, unsigned char __s15) __attribute__((__nothrow__, __const__));
int8x16 emscripten_uint8x16_splat(unsigned char __s) __attribute__((__nothrow__, __const__));
// n.b. No emscripten_uint8x16_check, not relevant for C/C++.
EM_INLINE uint8x16 emscripten_uint8x16_add(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a + __b; }
EM_INLINE uint8x16 emscripten_uint8x16_sub(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a - __b; }
EM_INLINE uint8x16 emscripten_uint8x16_mul(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a * __b; }
// n.b. No emscripten_uint8x16_div, division is only defined on floating point types.
// n.b. No emscripten_uint8x16_max, only defined on floating point types.
// n.b. No emscripten_uint8x16_min, only defined on floating point types.
// n.b. No emscripten_uint8x16_maxNum, only defined on floating point types.
// n.b. No emscripten_uint8x16_minNum, only defined on floating point types.
EM_INLINE uint8x16 emscripten_uint8x16_neg(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return -__a; }
// n.b. No emscripten_uint8x16_sqrt, only defined on floating point types.
// n.b. No emscripten_uint8x16_reciprocalApproximation, only defined on floating point types.
// n.b. No emscripten_uint8x16_reciprocalSqrtApproximation, only defined on floating point types.
// n.b. No emscripten_uint8x16_abs, only defined on floating point types.
EM_INLINE uint8x16 emscripten_uint8x16_and(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a & __b; }
EM_INLINE uint8x16 emscripten_uint8x16_xor(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a ^ __b; }
EM_INLINE uint8x16 emscripten_uint8x16_or(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a | __b; }
EM_INLINE uint8x16 emscripten_uint8x16_not(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return ~__a; }
EM_INLINE bool8x16 emscripten_uint8x16_lessThan(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a < __b; }
EM_INLINE bool8x16 emscripten_uint8x16_lessThanOrEqual(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a <= __b; }
EM_INLINE bool8x16 emscripten_uint8x16_greaterThan(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a > __b; }
EM_INLINE bool8x16 emscripten_uint8x16_greaterThanOrEqual(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a >= __b; }
EM_INLINE bool8x16 emscripten_uint8x16_equal(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a == __b; }
EM_INLINE bool8x16 emscripten_uint8x16_notEqual(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__, always_inline)) { return __a != __b; }
#define emscripten_uint8x16_anyTrue emscripten_bool8x16_anyTrue
#define emscripten_uint8x16_allTrue emscripten_bool8x16_allTrue
uint8x16 emscripten_uint8x16_select(bool8x16 __a, uint8x16 __b, uint8x16 __c) __attribute__((__nothrow__, __const__));
uint8x16 emscripten_uint8x16_addSaturate(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__));
uint8x16 emscripten_uint8x16_subSaturate(uint8x16 __a, uint8x16 __b) __attribute__((__nothrow__, __const__));
uint8x16 emscripten_uint8x16_shiftLeftByScalar(uint8x16 __a, int __count) __attribute__((__nothrow__, __const__));
uint8x16 emscripten_uint8x16_shiftRightByScalar(uint8x16 __a, int __count) __attribute__((__nothrow__, __const__));
EM_INLINE unsigned char emscripten_uint8x16_extractLane(uint8x16 __a, int __lane) __attribute__((__nothrow__, __const__, always_inline)) { return __a[__lane]; }
EM_INLINE uint8x16 emscripten_uint8x16_replaceLane(uint8x16 __a, int __lane, unsigned char __s) __attribute__((__nothrow__, __const__, always_inline)) { __a[__lane] = __s; return __a; }
void emscripten_uint8x16_store(const void *__p, uint8x16 __a) __attribute__((__nothrow__));
uint8x16 emscripten_uint8x16_load(const void *__p) __attribute__((__nothrow__, __pure__));
EM_INLINE uint8x16 emscripten_uint8x16_fromFloat64x2Bits(float64x2 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromInt32x4Bits(uint8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromUint32x4Bits(uint32x4 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromUint16x8Bits(uint16x8 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return (uint8x16)__a; }
EM_INLINE uint8x16 emscripten_uint8x16_fromInt8x16(int8x16 __a) __attribute__((__nothrow__, __const__, always_inline)) { return __builtin_convertvector(__a, uint8x16); }
uint8x16 emscripten_uint8x16_swizzle(uint8x16 __a, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7, int __lane8, int __lane9, int __lane10, int __lane11, int __lane12, int __lane13, int __lane14, int __lane15) __attribute__((__nothrow__, __const__));
uint8x16 emscripten_uint8x16_shuffle(uint8x16 __a, uint8x16 __b, int __lane0, int __lane1, int __lane2, int __lane3, int __lane4, int __lane5, int __lane6, int __lane7, int __lane8, int __lane9, int __lane10, int __lane11, int __lane12, int __lane13, int __lane14, int __lane15) __attribute__((__nothrow__, __const__));

// Bool64x2
int emscripten_bool64x2_anyTrue(bool64x2 __a) __attribute__((__nothrow__, __const__));
int emscripten_bool64x2_allTrue(bool64x2 __a) __attribute__((__nothrow__, __const__));

// Bool32x4
int emscripten_bool32x4_anyTrue(bool32x4 __a) __attribute__((__nothrow__, __const__));
int emscripten_bool32x4_allTrue(bool32x4 __a) __attribute__((__nothrow__, __const__));

// Bool16x8
int emscripten_bool16x8_anyTrue(bool16x8 __a) __attribute__((__nothrow__, __const__));
int emscripten_bool16x8_allTrue(bool16x8 __a) __attribute__((__nothrow__, __const__));

// Bool8x16
int emscripten_bool8x16_anyTrue(bool8x16 __a) __attribute__((__nothrow__, __const__));
int emscripten_bool8x16_allTrue(bool8x16 __a) __attribute__((__nothrow__, __const__));

#ifdef __cplusplus
}
#endif

#endif
