/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses SSE and SSE2 by calling undefined functions and prints the results.
// Use a diff tool to compare the results between platforms.

#include <stdio.h>
#include <inttypes.h>
#include <float.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#ifndef __cplusplus
#include <stdalign.h>
#endif

#include <emmintrin.h>
#define ENABLE_SSE2

void test_undefined_sse()
{
	// SSE Undefined instructions:
	#ifdef __EMSCRIPTEN__
	__m128 ps = _mm_undefined();
	#else
	__m128 ps = _mm_undefined_ps();
	#endif

	alignas(16) float p[4];
	_mm_store_ps((float*)p, ps);
	printf("%f %f %f %f\n", p[0], p[1], p[2], p[3]);
}

void test_undefined_sse2()
{
	// SSE2 Undefined instructions:
	__m128i si128 = _mm_undefined_si128();
	__m128d pd = _mm_undefined_pd();

	alignas(16) uint32_t s[4];
	alignas(16) double p[4];
	_mm_store_si128((__m128i*)s, si128);
	_mm_store_pd((double*)p, pd);
	printf("%x %x %x %x\n", s[0], s[1], s[2], s[3]);
	printf("%f %f %f %f\n", p[0], p[1], p[2], p[3]);
}

int main()
{
	test_undefined_sse();
	test_undefined_sse2();
}
