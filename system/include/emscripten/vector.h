#ifndef __emscripten_vector_h__
#define __emscripten_vector_h__

#ifdef __cplusplus
extern "C" {
#endif

// Support for the JS SIMD API proposal, https://github.com/johnmccutchan/ecmascript_simd

typedef float float32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef double float64x2 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned long long int64x2 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned int int32x4 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned short int16x8 __attribute__((__vector_size__(16), __may_alias__));
typedef unsigned char int8x16 __attribute__((__vector_size__(16), __may_alias__));

unsigned int emscripten_float32x4_signmask(float32x4 __x) __attribute__((__nothrow__, __const__));

float32x4 emscripten_float32x4_min(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_minNum(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_max(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_maxNum(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_abs(float32x4 __a) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_sqrt(float32x4 __a) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_reciprocalApproximation(float32x4 __a) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_reciprocalSqrtApproximation(float32x4 __a) __attribute__((__nothrow__, __const__));
inline int32x4 emscripten_float32x4_lessThan(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__)) { return __a < __b; }
inline int32x4 emscripten_float32x4_lessThanOrEqual(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__)) { return __a <= __b; }
inline int32x4 emscripten_float32x4_equal(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__)) { return __a == __b; }
inline int32x4 emscripten_float32x4_notEqual(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__)) { return __a != __b; }
inline int32x4 emscripten_float32x4_greaterThanOrEqual(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__)) { return __a >= __b; }
inline int32x4 emscripten_float32x4_greaterThan(float32x4 __a, float32x4 __b) __attribute__((__nothrow__, __const__)) { return __a > __b; }
float32x4 emscripten_float32x4_select(int32x4 __a, float32x4 __b, float32x4 __c) __attribute__((__nothrow__, __const__));

inline int32x4 emscripten_int32x4_and(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__)) { return __a & __b; }
inline int32x4 emscripten_int32x4_or(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__)) { return __a | __b; }
inline int32x4 emscripten_int32x4_xor(int32x4 __a, int32x4 __b) __attribute__((__nothrow__, __const__)) { return __a ^ __b; }
inline int32x4 emscripten_int32x4_not(int32x4 __a) __attribute__((__nothrow__, __const__)) { return ~__a; }
inline int32x4 emscripten_int32x4_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__)) { return (int32x4)__a; }
inline int32x4 emscripten_int32x4_fromFloat32x4(float32x4 __a) __attribute__((__nothrow__, __const__)) { return __builtin_convertvector(__a, int32x4); }

float64x2 emscripten_float64x2_min(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_minNum(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_max(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_maxNum(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_abs(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_sqrt(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_reciprocalApproximation(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_reciprocalSqrtApproximation(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_fromInt8x16Bits(int8x16 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_fromInt16x8Bits(int16x8 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_fromInt32x4Bits(int32x4 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_fromInt32x4(int32x4 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_fromFloat32x4(float32x4 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_fromFloat32x4Bits(float32x4 __a) __attribute__((__nothrow__, __const__));
int32x4 emscripten_float64x2_lessThan(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
int32x4 emscripten_float64x2_lessThanOrEqual(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
int32x4 emscripten_float64x2_equal(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
int32x4 emscripten_float64x2_notEqual(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
int32x4 emscripten_float64x2_greaterThanOrEqual(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
int32x4 emscripten_float64x2_greaterThan(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_and(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_or(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_xor(float64x2 __a, float64x2 __b) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_not(float64x2 __a) __attribute__((__nothrow__, __const__));
float64x2 emscripten_float64x2_select(int32x4 __a, float64x2 __b, float64x2 __c) __attribute__((__nothrow__, __const__));

int32x4 emscripten_int32x4_select(int32x4 __a, int32x4 __b, int32x4 __c) __attribute__((__nothrow__, __const__));


int8x16 emscripten_int8x16_shiftLeftByScalar(int8x16 __a, int __count) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_shiftRightLogicalByScalar(int8x16 __a, int __count) __attribute__((__nothrow__, __const__));
int8x16 emscripten_int8x16_shiftRightArithmeticByScalar(int8x16 __a, int __count) __attribute__((__nothrow__, __const__));

int16x8 emscripten_int16x8_shiftLeftByScalar(int16x8 __a, int __count) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_shiftRightLogicalByScalar(int16x8 __a, int __count) __attribute__((__nothrow__, __const__));
int16x8 emscripten_int16x8_shiftRightArithmeticByScalar(int16x8 __a, int __count) __attribute__((__nothrow__, __const__));

int32x4 emscripten_int32x4_shiftLeftByScalar(int32x4 __a, int __count) __attribute__((__nothrow__, __const__));
int32x4 emscripten_int32x4_shiftRightLogicalByScalar(int32x4 __a, int __count) __attribute__((__nothrow__, __const__));
int32x4 emscripten_int32x4_shiftRightArithmeticByScalar(int32x4 __a, int __count) __attribute__((__nothrow__, __const__));

float32x4 emscripten_float32x4_load1(const void *__p) __attribute__((__nothrow__, __const__));
float32x4 emscripten_float32x4_load2(const void *__p) __attribute__((__nothrow__, __const__));
void emscripten_float32x4_store1(const void *__p, float32x4 __a) __attribute__((__nothrow__));
void emscripten_float32x4_store2(const void *__p, float32x4 __a) __attribute__((__nothrow__));

#ifdef __cplusplus
}
#endif

#endif
