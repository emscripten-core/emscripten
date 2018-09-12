/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <xmmintrin.h>
#include <stdio.h>
#include <string>

std::string tostr(__m128 m)
{
    unsigned int u[4];
    _mm_store_ps((float*)u, m);
    char str[256];
    sprintf(str, "[%X,%X,%X,%X]", u[3], u[2], u[1], u[0]);
    return str;
}

int main()
{
    unsigned int u[4] = { 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU };
    __m128 m1 = _mm_load_ps((float*)&u);
    printf("%s\n", tostr(m1).c_str());
}
