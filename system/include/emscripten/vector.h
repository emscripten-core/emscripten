
// Support for the JS SIMD API proposal, https://github.com/johnmccutchan/ecmascript_simd

typedef float float32x4 __attribute__((__vector_size__(16)));
typedef unsigned int uint32x4 __attribute__((__vector_size__(16)));

#ifdef __cplusplus
extern "C" {
#endif

unsigned int emscripten_float32x4_signmask(float32x4 x);

#ifdef __cplusplus
}
#endif

