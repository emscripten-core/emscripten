/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <xmmintrin.h>
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
  int dst_int_scalar = 0;
  for(int i = 0; i < N; ++i) src_int[i] = rand();
  for(int i = 0; i < N; ++i) src2_int[i] = rand();

  float scalarTime = 0.f;

  // Benchmarks start:
  SETCHART("load");
  START();
    for(int i = 0; i < N; ++i)
      dst_flt[i] = src_flt[i];
  ENDSCALAR(checksum_dst(dst_flt), "scalar");

  LOAD_STORE_F("_mm_load_ps", _mm_load_ps, 0, _mm_store_ps, float*, 0, 4);
  LOAD_STORE_F("_mm_load_ps1", _mm_load_ps1, 1, _mm_store_ps, float*, 0, 4);
  LOAD_STORE_F("_mm_load_ss", _mm_load_ss, 1, _mm_store_ps, float*, 0, 4);
  LOAD_STORE_F("_mm_load1_ps", _mm_load1_ps, 1, _mm_store_ps, float*, 0, 4);

  __m128 tempReg = _mm_set_ps(1.f, 2.f, 3.f, 4.f);
  LOAD_STORE_M64("_mm_loadh_pi", tempReg, _mm_loadh_pi, __m64*, 1, _mm_store_ps, float*, 0, 4);
  LOAD_STORE_M64("_mm_loadl_pi", tempReg, _mm_loadh_pi, __m64*, 1, _mm_store_ps, float*, 0, 4);

  LOAD_STORE_F("_mm_loadr_ps", _mm_loadr_ps, 0, _mm_store_ps, float*, 0, 4);
  LOAD_STORE_F("_mm_loadu_ps", _mm_loadu_ps, 1, _mm_store_ps, float*, 0, 4);

  SETCHART("set");
  SET_STORE_F("_mm_set_ps", _mm_set_ps(src_flt[i+2], src_flt[i+1], src_flt[i+5], src_flt[i+0]));
  SET_STORE_F("_mm_set_ps1", _mm_set_ps1(src_flt[i]));
  SET_STORE_F("_mm_set_ss", _mm_set_ss(src_flt[i]));
  SET_STORE_F("_mm_set1_ps", _mm_set1_ps(src_flt[i]));
  SET_STORE_F("_mm_setr_ps", _mm_set_ps(src_flt[i+2], src_flt[i+1], src_flt[i+5], src_flt[i+0]));
  SET_STORE_F("_mm_setzero_ps", _mm_setzero_ps());

  SETCHART("move");
  SET_STORE_F("_mm_move_ss", _mm_move_ss(_mm_load_ps(src_flt+i), _mm_load_ps(src2_flt+i)));
  SET_STORE_F("_mm_movehl_ps", _mm_movehl_ps(_mm_load_ps(src_flt+i), _mm_load_ps(src2_flt+i)));
  SET_STORE_F("_mm_movelh_ps", _mm_movelh_ps(_mm_load_ps(src_flt+i), _mm_load_ps(src2_flt+i)));

  SETCHART("movemask");
  START(); for(int i = 0; i < N; i += 4) { int movemask = ((unsigned int)src_flt[i] >> 31) | (((unsigned int)src_flt[i+1] >> 30) & 2)  | (((unsigned int)src_flt[i+2] >> 29) & 4)  | (((unsigned int)src_flt[i+3] >> 28) & 8); dst_int_scalar += movemask; } ENDSCALAR(dst_int_scalar, "scalar movemask");
  UNARYOP_i_F("_mm_movemask_ps", _mm_movemask_ps(_mm_load_ps(src_flt+i)));

  SETCHART("store");
  LOAD_STORE_F("_mm_store_ps", _mm_load_ps, 0, _mm_store_ps, float*, 0, 4);
  LOAD_STORE_F("_mm_store_ps1", _mm_load_ps, 0, _mm_store_ps1, float*, 0, 4);
  LOAD_STORE_F("_mm_store_ss", _mm_load_ps, 0, _mm_store_ss, float*, 1, 4);
  LOAD_STORE_64_F("_mm_storeh_pi", _mm_load_ps, 0, _mm_storeh_pi, 1, 4);
  LOAD_STORE_64_F("_mm_storel_pi", _mm_load_ps, 0, _mm_storel_pi, 1, 4);
  LOAD_STORE_F("_mm_storer_ps", _mm_load_ps, 0, _mm_storer_ps, float*, 0, 4);
  LOAD_STORE_F("_mm_storeu_ps", _mm_load_ps, 0, _mm_storeu_ps, float*, 1, 4);
  LOAD_STORE_F("_mm_stream_ps", _mm_load_ps, 0, _mm_stream_ps, float*, 0, 4);

  SETCHART("arithmetic");
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] += src2_flt[0]; dst_flt[1] += src2_flt[1]; dst_flt[2] += src2_flt[2]; dst_flt[3] += src2_flt[3]; } ENDSCALAR(checksum_dst(dst_flt), "scalar add");
  BINARYOP_F_FF("_mm_add_ps", _mm_add_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_add_ss", _mm_add_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] /= src2_flt[0]; dst_flt[1] /= src2_flt[1]; dst_flt[2] /= src2_flt[2]; dst_flt[3] /= src2_flt[3]; } ENDSCALAR(checksum_dst(dst_flt), "scalar div");
  BINARYOP_F_FF("_mm_div_ps", _mm_div_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_div_ss", _mm_div_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] *= src2_flt[0]; dst_flt[1] *= src2_flt[1]; dst_flt[2] *= src2_flt[2]; dst_flt[3] *= src2_flt[3]; } ENDSCALAR(checksum_dst(dst_flt), "scalar mul");
  BINARYOP_F_FF("_mm_mul_ps", _mm_mul_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_mul_ss", _mm_mul_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] -= src2_flt[0]; dst_flt[1] -= src2_flt[1]; dst_flt[2] -= src2_flt[2]; dst_flt[3] -= src2_flt[3]; } ENDSCALAR(checksum_dst(dst_flt), "scalar sub");
  BINARYOP_F_FF("_mm_sub_ps", _mm_sub_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_sub_ss", _mm_sub_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  SETCHART("roots");
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = 1.f / dst_flt[0]; dst_flt[1] = 1.f / dst_flt[1]; dst_flt[2] = 1.f / dst_flt[2]; dst_flt[3] = 1.f / dst_flt[3]; } ENDSCALAR(checksum_dst(dst_flt), "scalar rcp");
  UNARYOP_F_F("_mm_rcp_ps", _mm_rcp_ps, _mm_load_ps(src_flt));
  UNARYOP_F_F("_mm_rcp_ss", _mm_rcp_ss, _mm_load_ps(src_flt));

  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = 1.f / sqrtf(dst_flt[0]); dst_flt[1] = 1.f / sqrtf(dst_flt[1]); dst_flt[2] = 1.f / sqrtf(dst_flt[2]); dst_flt[3] = 1.f / sqrtf(dst_flt[3]); } ENDSCALAR(checksum_dst(dst_flt), "scalar rsqrt");
  UNARYOP_F_F("_mm_rsqrt_ps", _mm_rsqrt_ps, _mm_load_ps(src_flt));
  UNARYOP_F_F("_mm_rsqrt_ss", _mm_rsqrt_ss, _mm_load_ps(src_flt));

  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = sqrtf(dst_flt[0]); dst_flt[1] = sqrtf(dst_flt[1]); dst_flt[2] = sqrtf(dst_flt[2]); dst_flt[3] = sqrtf(dst_flt[3]); } ENDSCALAR(checksum_dst(dst_flt), "scalar sqrt");
  UNARYOP_F_F("_mm_sqrt_ps", _mm_sqrt_ps, _mm_load_ps(src_flt));
  UNARYOP_F_F("_mm_sqrt_ss", _mm_sqrt_ss, _mm_load_ps(src_flt));

  SETCHART("logical");
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = ucastf(fcastu(dst_flt[0]) & fcastu(src2_flt[0])); dst_flt[1] = ucastf(fcastu(dst_flt[1]) & fcastu(src2_flt[1])); dst_flt[2] = ucastf(fcastu(dst_flt[2]) & fcastu(src2_flt[2])); dst_flt[3] = ucastf(fcastu(dst_flt[3]) & fcastu(src2_flt[3])); } ENDSCALAR(checksum_dst(dst_flt), "scalar and");
  BINARYOP_F_FF("_mm_and_ps", _mm_and_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = ucastf((~fcastu(dst_flt[0])) & fcastu(src2_flt[0])); dst_flt[1] = ucastf((~fcastu(dst_flt[1])) & fcastu(src2_flt[1])); dst_flt[2] = ucastf((~fcastu(dst_flt[2])) & fcastu(src2_flt[2])); dst_flt[3] = ucastf((~fcastu(dst_flt[3])) & fcastu(src2_flt[3])); } ENDSCALAR(checksum_dst(dst_flt), "scalar andnot");
  BINARYOP_F_FF("_mm_andnot_ps", _mm_andnot_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = ucastf(fcastu(dst_flt[0]) | fcastu(src2_flt[0])); dst_flt[1] = ucastf(fcastu(dst_flt[1]) | fcastu(src2_flt[1])); dst_flt[2] = ucastf(fcastu(dst_flt[2]) | fcastu(src2_flt[2])); dst_flt[3] = ucastf(fcastu(dst_flt[3]) | fcastu(src2_flt[3])); } ENDSCALAR(checksum_dst(dst_flt), "scalar or");
  BINARYOP_F_FF("_mm_or_ps", _mm_or_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = ucastf(fcastu(dst_flt[0]) ^ fcastu(src2_flt[0])); dst_flt[1] = ucastf(fcastu(dst_flt[1]) ^ fcastu(src2_flt[1])); dst_flt[2] = ucastf(fcastu(dst_flt[2]) ^ fcastu(src2_flt[2])); dst_flt[3] = ucastf(fcastu(dst_flt[3]) ^ fcastu(src2_flt[3])); } ENDSCALAR(checksum_dst(dst_flt), "scalar xor");
  BINARYOP_F_FF("_mm_xor_ps", _mm_xor_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  SETCHART("cmp");
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (dst_flt[0] == src2_flt[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (dst_flt[1] == src2_flt[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (dst_flt[2] == src2_flt[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (dst_flt[3] == src2_flt[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmp==");
  BINARYOP_F_FF("_mm_cmpeq_ps", _mm_cmpeq_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmpeq_ss", _mm_cmpeq_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (dst_flt[0] >= src2_flt[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (dst_flt[1] >= src2_flt[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (dst_flt[2] >= src2_flt[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (dst_flt[3] >= src2_flt[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmp>=");
  BINARYOP_F_FF("_mm_cmpge_ps", _mm_cmpge_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmpge_ss", _mm_cmpge_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (dst_flt[0] > src2_flt[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (dst_flt[1] > src2_flt[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (dst_flt[2] > src2_flt[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (dst_flt[3] > src2_flt[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmp>");
  BINARYOP_F_FF("_mm_cmpgt_ps", _mm_cmpgt_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmpgt_ss", _mm_cmpgt_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (dst_flt[0] <= src2_flt[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (dst_flt[1] <= src2_flt[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (dst_flt[2] <= src2_flt[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (dst_flt[3] <= src2_flt[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmp<=");
  BINARYOP_F_FF("_mm_cmple_ps", _mm_cmple_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmple_ss", _mm_cmple_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (dst_flt[0] < src2_flt[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (dst_flt[1] < src2_flt[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (dst_flt[2] < src2_flt[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (dst_flt[3] < src2_flt[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmp<");
  BINARYOP_F_FF("_mm_cmplt_ps", _mm_cmplt_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmplt_ss", _mm_cmplt_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (!Isnan(dst_flt[0]) && !Isnan(src2_flt[0])) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (!Isnan(dst_flt[1]) && !Isnan(src2_flt[1])) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (!Isnan(dst_flt[2]) && !Isnan(src2_flt[2])) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (!Isnan(dst_flt[3]) && !Isnan(src2_flt[3])) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmpord");
  BINARYOP_F_FF("_mm_cmpord_ps", _mm_cmpord_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmpord_ss", _mm_cmpord_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = (Isnan(dst_flt[0]) || Isnan(src2_flt[0])) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[1] = (Isnan(dst_flt[1]) || Isnan(src2_flt[1])) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[2] = (Isnan(dst_flt[2]) || Isnan(src2_flt[2])) ? ucastf(0xFFFFFFFFU) : 0.f; dst_flt[3] = (Isnan(dst_flt[3]) || Isnan(src2_flt[3])) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_flt), "scalar cmpunord");
  BINARYOP_F_FF("_mm_cmpunord_ps", _mm_cmpunord_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_cmpunord_ss", _mm_cmpunord_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  SETCHART("max");
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = Max(dst_flt[0], src2_flt[0]); dst_flt[1] = Max(dst_flt[1], src2_flt[1]); dst_flt[2] = Max(dst_flt[2], src2_flt[2]); dst_flt[3] = Max(dst_flt[3], src2_flt[3]); } ENDSCALAR(checksum_dst(dst_flt), "scalar max");
  BINARYOP_F_FF("_mm_max_ps", _mm_max_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_max_ss", _mm_max_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = Min(dst_flt[0], src2_flt[0]); dst_flt[1] = Min(dst_flt[1], src2_flt[1]); dst_flt[2] = Min(dst_flt[2], src2_flt[2]); dst_flt[3] = Min(dst_flt[3], src2_flt[3]); } ENDSCALAR(checksum_dst(dst_flt), "scalar min");
  BINARYOP_F_FF("_mm_min_ps", _mm_min_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  BINARYOP_F_FF("_mm_min_ss", _mm_min_ss, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  SETCHART("shuffle");
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[3] = dst_flt[1]; dst_flt[2] = dst_flt[0]; dst_flt[1] = src2_flt[3]; dst_flt[0] = src2_flt[2]; } ENDSCALAR(checksum_dst(dst_flt), "scalar shuffle");

  START();
    __m128 o0 = _mm_load_ps(src_flt);
    __m128 o1 = _mm_load_ps(src2_flt);
    for(int i = 0; i < N; i += 4)
      o0 = _mm_shuffle_ps(o0, o1, _MM_SHUFFLE(1, 0, 3, 2));
    _mm_store_ps(dst_flt, o0);
  END(checksum_dst(dst_flt), "_mm_shuffle_ps");

  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[0] = dst_flt[2]; dst_flt[1] = src2_flt[2]; dst_flt[2] = dst_flt[3]; dst_flt[3] = src2_flt[3]; } ENDSCALAR(checksum_dst(dst_flt), "scalar unpackhi_ps");
  BINARYOP_F_FF("_mm_unpackhi_ps", _mm_unpackhi_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));
  START(); dst_flt[0] = src_flt[0]; dst_flt[1] = src_flt[1]; dst_flt[2] = src_flt[2]; dst_flt[3] = src_flt[3]; for(int i = 0; i < N; ++i) { dst_flt[2] = dst_flt[1]; dst_flt[1] = dst_flt[0]; dst_flt[0] = src2_flt[0]; dst_flt[3] = src2_flt[1]; } ENDSCALAR(checksum_dst(dst_flt), "scalar unpacklo_ps");
  BINARYOP_F_FF("_mm_unpacklo_ps", _mm_unpacklo_ps, _mm_load_ps(src_flt), _mm_load_ps(src2_flt));

  // Benchmarks end:
  printf("]}\n");
}
