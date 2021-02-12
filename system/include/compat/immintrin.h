/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#ifndef __emscripten_immintrin_h__
#define __emscripten_immintrin_h__

#ifdef __AVX__
#include <avxintrin.h>
#endif

#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#endif /* __emscripten_immintrin_h__ */
