#ifndef __emscripten_immintrin_h__
#define __emscripten_immintrin_h__

// Include all Intel specific intrinsics.

#if __SSE__
#include <xmmintrin.h>
#else
#warning immintrin.h included without SIMD.js support enabled.
#endif

#if __SSE2__
#include <emmintrin.h>
#endif

#if __SSE3__
#include <pmmintrin.h>
#endif

#if __SSSE3__
#include <tmmintrin.h>
#endif

#if __SSE4_1__
#include <smmintrin.h>
#endif

#endif
