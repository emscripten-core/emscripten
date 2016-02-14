#ifndef __X86INTRIN_H
#define __X86INTRIN_H

// x86intrin.h is the standard include-all for all supported intrinsics.

#if __SSE__
#include <xmmintrin.h>
#else
#warning x86intrin.h included without SIMD.js support enabled.
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
