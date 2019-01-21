/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emmintrin.h>
#include <stdio.h>
#include <time.h>

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

int main()
{
	int a = _mm_movemask_ps(_mm_set_epi32(-1, -1, -1, -1));
	printf("%d\n", a);
}
