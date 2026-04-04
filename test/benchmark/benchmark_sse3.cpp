/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <pmmintrin.h>
#include "benchmark_sse.h"

int main() {
  printf ("{ \"workload\": %u, \"results\": [\n", N);

  float *src_flt = alloc_float_buffer();
  float *src2_flt = alloc_float_buffer();
  float *dst_flt = alloc_float_buffer();
  for(int i = 0; i < N; ++i) src_flt[i] = (float)(1.0 + (double)rand() / RAND_MAX);
  for(int i = 0; i < N; ++i) src2_flt[i] = (float)(1.0 + (double)rand() / RAND_MAX);

  double *src_dbl = alloc_double_buffer();
  double *src2_dbl = alloc_double_buffer();
  double *dst_dbl = alloc_double_buffer();
  for(int i = 0; i < N; ++i) src_dbl[i] = 1.0 + (double)rand() / RAND_MAX;
  for(int i = 0; i < N; ++i) src2_dbl[i] = 1.0 + (double)rand() / RAND_MAX;

  int *src_int = alloc_int_buffer();
  int *src2_int = alloc_int_buffer();
  int *dst_int = alloc_int_buffer();
  for(int i = 0; i < N; ++i) src_int[i] = rand();
  for(int i = 0; i < N; ++i) src2_int[i] = rand();

  float scalarTime = 0.f;

  // Benchmarks start:
  SETCHART("load");
  LOAD_STORE_I("_mm_lddqu_si128", _mm_lddqu_si128, 0, _mm_store_si128, 0, 4);
  LOAD_STORE_D("_mm_loaddup_pd", _mm_loaddup_pd, 0, _mm_store_pd, double*, 0, 2);

  SETCHART("double arithmetic");
  BINARYOP_D_DD("_mm_addsub_pd", _mm_addsub_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_hadd_pd", _mm_hadd_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_hsub_pd", _mm_hsub_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));

  SETCHART("double swizzle");
  UNARYOP_D_D("_mm_movedup_pd", _mm_movedup_pd, _mm_load_pd(src_dbl));

  SETCHART("float arithmetic");
  BINARYOP_F_FF("_mm_addsub_ps", _mm_addsub_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_hadd_ps", _mm_hadd_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_hsub_ps", _mm_hsub_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  SETCHART("float swizzle");
  UNARYOP_F_F("_mm_movehdup_ps", _mm_movehdup_ps, _mm_load_ps(src_flt));
  UNARYOP_F_F("_mm_moveldup_ps", _mm_moveldup_ps, _mm_load_ps(src_flt));

  // Benchmarks end:
  printf("]}\n");
}
