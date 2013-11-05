
// Support for the JS SIMD API proposal, https://github.com/johnmccutchan/ecmascript_simd

typedef float float32x4 __attribute__((__vector_size__(16)));
typedef unsigned int uint32x4 __attribute__((__vector_size__(16)));

#ifdef __cplusplus
extern "C" {
#endif

int emscripten_float32x4_signmask(float32x4 a);

float32x4 emscripten_float32x4_min(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_max(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_sqrt(float32x4 a);
float32x4 emscripten_float32x4_lessThan(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_lessThanOrEqual(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_equal(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_greaterThanOrEqual(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_greaterThan(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_and(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_andNot(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_or(float32x4 a, float32x4 b);
float32x4 emscripten_float32x4_xor(float32x4 a, float32x4 b);

float32x4 emscripten_uint32x4_bitsToFloat32x4(uint32x4 a);
float32x4 emscripten_uint32x4_toFloat32x4(uint32x4 a);
uint32x4 emscripten_float32x4_bitsToUint32x4(float32x4 a);
uint32x4 emscripten_float32x4_toUint32x4(float32x4 a);

#ifdef __cplusplus
}
#endif

