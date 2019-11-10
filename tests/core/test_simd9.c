/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <xmmintrin.h>
#include <stdio.h>
#include <time.h>

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

int main()
{
	__m128 a = always_true() ? _mm_set1_ps(0.f) : _mm_set1_ps(2.f);
	printf("%d %d %d %d\n", (int)a[0], (int)a[1], (int)a[2], (int)a[3]);
}
