/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <wasm_simd128.h>
#include <xmmintrin.h>

int main() {
  __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
  __m128 b = _mm_rcp_ps(a);
  return (int)wasm_f32x4_extract_lane((v128_t)b, 0);
}
