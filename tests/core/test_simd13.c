/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <xmmintrin.h>
#include <stdio.h>

class float4
{
public:
	float4(){}
	float4(__m128 in) { m128 = in; }
	operator __m128() { return m128; }
	operator const __m128() const { return m128; }
	__m128 m128;
};

int main()
{
	float4 v = _mm_set_ps(3, 2, 1, 0);
	float4 w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
	printf("%d\n", (int)_mm_cvtss_f32(w));
}
