/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>

void print128_num(__m128i var) 
{
    int32_t *v32val = (int32_t*) &var;
    printf("%.8d %.8d %.8d %.8d\n", v32val[3], v32val[2], v32val[1], v32val[0]);
}

int main() {
  __m128i var = _mm_set_epi64x(-1588454185101182573, 437384867522774919);
  print128_num(var);
}
