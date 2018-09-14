/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <xmmintrin.h>

void __attribute__((noinline)) foo(__m128 vec, __m128 &outLength)
{
    outLength = vec;
}

int main()
{
    puts("before");
    __m128 x;
    foo(_mm_set1_ps(1.f), x);
    printf("%.2f", x[0]);
}

