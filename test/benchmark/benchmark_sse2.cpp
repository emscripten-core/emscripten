/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <emmintrin.h>
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
  START();
    for(int i = 0; i < N; ++i)
      dst_dbl[i] = src_dbl[i];
  ENDSCALAR(checksum_dst(dst_dbl), "scalar");

  LOAD_STORE_D("_mm_load_pd", _mm_load_pd, 0, _mm_store_pd, double*, 0, 2);
  LOAD_STORE_D("_mm_load_pd1", _mm_load_pd1, 1, _mm_store_pd, double*, 0, 2);
  LOAD_STORE_D("_mm_load_sd", _mm_load_sd, 1, _mm_store_pd, double*, 0, 2);
  // _mm_load_si128
  LOAD_STORE_D("_mm_load1_pd", _mm_load1_pd, 1, _mm_store_pd, double*, 0, 2);

  __m128d tempReg = _mm_set_pd(1.0, 2.0);
  LOAD_STORE_M64("_mm_loadh_pd", tempReg, _mm_loadh_pd, double*, 1, _mm_store_pd, double*, 0, 2);
  // _mm_loadl_epi64
  LOAD_STORE_M64("_mm_loadl_pd", tempReg, _mm_loadh_pd, double*, 1, _mm_store_pd, double*, 0, 2);

  LOAD_STORE_D("_mm_loadr_pd", _mm_loadr_pd, 0, _mm_store_pd, double*, 0, 2);
  LOAD_STORE_D("_mm_loadu_pd", _mm_loadu_pd, 1, _mm_store_pd, double*, 0, 2);
  // _mm_loadu_si128

  SETCHART("set");
/*  _mm_set_epi16
  _mm_set_epi32
  _mm_set_epi64
  _mm_set_epi64x
  _mm_set_epi8 */
  SET_STORE_D("_mm_set_pd", _mm_set_pd(src_dbl[i+2], src_dbl[i+0]));
  //SET_STORE_D("_mm_set_pd1", _mm_set_pd1(src_dbl[i]));
  SET_STORE_D("_mm_set_sd", _mm_set_sd(src_dbl[i]));
/*  _mm_set1_epi16
  _mm_set1_epi32
  _mm_set1_epi64
  _mm_set1_epi64x
  _mm_set1_epi8 */
  SET_STORE_D("_mm_set1_pd", _mm_set1_pd(src_dbl[i]));
/*  _mm_setr_epi16
  _mm_setr_epi32
  _mm_setr_epi64
  _mm_setr_epi8 */
  SET_STORE_D("_mm_setr_pd", _mm_set_pd(src_dbl[i+2], src_dbl[i+0]));
  SET_STORE_D("_mm_setzero_pd", _mm_setzero_pd());
//  _mm_setzero_si128

  SETCHART("move");
  // _mm_move_epi64
  SET_STORE_D("_mm_move_sd", _mm_move_sd(_mm_load_pd(src_dbl+i), _mm_load_pd(src2_dbl+i)));

  SETCHART("store");
  // _mm_maskmoveu_si128
  LOAD_STORE_D("_mm_store_pd", _mm_load_pd, 0, _mm_store_pd, double*, 0, 2);
//  LOAD_STORE_D("_mm_store_pd1", _mm_load_pd, 0, _mm_store_pd1, double*, 0);
  LOAD_STORE_D("_mm_store_sd", _mm_load_pd, 0, _mm_store_sd, double*, 1, 2);
  // _mm_store_si128
  // _mm_store1_pd
  LOAD_STORE_64_D("_mm_storeh_pi", _mm_load_pd, 0, _mm_storeh_pi, 1, 2);
  // _mm_storel_epi64
  LOAD_STORE_64_D("_mm_storel_pi", _mm_load_pd, 0, _mm_storel_pi, 1, 2);
  LOAD_STORE_D("_mm_storer_pd", _mm_load_pd, 0, _mm_storer_pd, double*, 0, 2);
  LOAD_STORE_D("_mm_storeu_pd", _mm_load_pd, 0, _mm_storeu_pd, double*, 1, 2);
  // _mm_storeu_si128
  LOAD_STORE_D("_mm_stream_pd", _mm_load_pd, 0, _mm_stream_pd, double*, 0, 2);
  // _mm_stream_si128
  // _mm_stream_si32
  // _mm_stream_si64

  SETCHART("arithmetic");
  // _mm_add_epi16
  // _mm_add_epi32
  // _mm_add_epi64
  // _mm_add_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] += src2_dbl[0]; dst_dbl[1] += src2_dbl[1]; dst_dbl[2] += src2_dbl[2]; dst_dbl[3] += src2_dbl[3]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar add");
  BINARYOP_D_DD("_mm_add_pd", _mm_add_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_add_sd", _mm_add_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_adds_epi16
  // _mm_adds_epi8
  // _mm_adds_epu16
  // _mm_adds_epu8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] /= src2_dbl[0]; dst_dbl[1] /= src2_dbl[1]; dst_dbl[2] /= src2_dbl[2]; dst_dbl[3] /= src2_dbl[3]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar div");
  BINARYOP_D_DD("_mm_div_pd", _mm_div_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_div_sd", _mm_div_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_madd_epi16
  // _mm_mul_epu32
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] *= src2_dbl[0]; dst_dbl[1] *= src2_dbl[1]; dst_dbl[2] *= src2_dbl[2]; dst_dbl[3] *= src2_dbl[3]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar mul");
  BINARYOP_D_DD("_mm_mul_pd", _mm_mul_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_mul_sd", _mm_mul_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_mulhi_epi16
  // _mm_mulhi_epu16
  // _mm_mullo_epi16
  // _mm_sad_epu8
  // _mm_sub_epi16
  // _mm_sub_epi32
  // _mm_sub_epi64
  // _mm_sub_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] -= src2_dbl[0]; dst_dbl[1] -= src2_dbl[1]; dst_dbl[2] -= src2_dbl[2]; dst_dbl[3] -= src2_dbl[3]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar sub");
  BINARYOP_D_DD("_mm_sub_pd", _mm_sub_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_sub_sd", _mm_sub_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_subs_epi16
  // _mm_subs_epi8
  // _mm_subs_epu16
  // _mm_subs_epu8

  SETCHART("roots");
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = sqrt(dst_dbl[0]); dst_dbl[1] = sqrt(dst_dbl[1]); dst_dbl[2] = sqrt(dst_dbl[2]); dst_dbl[3] = sqrt(dst_dbl[3]); } ENDSCALAR(checksum_dst(dst_dbl), "scalar sqrt");
  UNARYOP_D_D("_mm_sqrt_pd", _mm_sqrt_pd, _mm_load_pd(src_dbl));
//  UNARYOP_D_D("_mm_sqrt_sd", _mm_sqrt_sd, _mm_load_pd(src_dbl));

  SETCHART("logical");
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = ucastd(dcastu(dst_dbl[0]) & dcastu(src2_dbl[0])); dst_dbl[1] = ucastd(dcastu(dst_dbl[1]) & dcastu(src2_dbl[1])); dst_dbl[2] = ucastd(dcastu(dst_dbl[2]) & dcastu(src2_dbl[2])); dst_dbl[3] = ucastd(dcastu(dst_dbl[3]) & dcastu(src2_dbl[3])); } ENDSCALAR(checksum_dst(dst_dbl), "scalar and");
  BINARYOP_D_DD("_mm_and_pd", _mm_and_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_and_si128
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = ucastd((~dcastu(dst_dbl[0])) & dcastu(src2_dbl[0])); dst_dbl[1] = ucastd((~dcastu(dst_dbl[1])) & dcastu(src2_dbl[1])); dst_dbl[2] = ucastd((~dcastu(dst_dbl[2])) & dcastu(src2_dbl[2])); dst_dbl[3] = ucastd((~dcastu(dst_dbl[3])) & dcastu(src2_dbl[3])); } ENDSCALAR(checksum_dst(dst_dbl), "scalar andnot");
  BINARYOP_D_DD("_mm_andnot_pd", _mm_andnot_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_andnot_si128
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = ucastd(dcastu(dst_dbl[0]) | dcastu(src2_dbl[0])); dst_dbl[1] = ucastd(dcastu(dst_dbl[1]) | dcastu(src2_dbl[1])); dst_dbl[2] = ucastd(dcastu(dst_dbl[2]) | dcastu(src2_dbl[2])); dst_dbl[3] = ucastd(dcastu(dst_dbl[3]) | dcastu(src2_dbl[3])); } ENDSCALAR(checksum_dst(dst_dbl), "scalar or");
  BINARYOP_D_DD("_mm_or_pd", _mm_or_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_or_si128
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = ucastd(dcastu(dst_dbl[0]) ^ dcastu(src2_dbl[0])); dst_dbl[1] = ucastd(dcastu(dst_dbl[1]) ^ dcastu(src2_dbl[1])); dst_dbl[2] = ucastd(dcastu(dst_dbl[2]) ^ dcastu(src2_dbl[2])); dst_dbl[3] = ucastd(dcastu(dst_dbl[3]) ^ dcastu(src2_dbl[3])); } ENDSCALAR(checksum_dst(dst_dbl), "scalar xor");
  BINARYOP_D_DD("_mm_xor_pd", _mm_xor_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_xor_si128

  SETCHART("cmp");
  // _mm_cmpeq_epi16
  // _mm_cmpeq_epi32
  // _mm_cmpeq_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (dst_dbl[0] == src2_dbl[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (dst_dbl[1] == src2_dbl[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (dst_dbl[2] == src2_dbl[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (dst_dbl[3] == src2_dbl[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmp==");
  BINARYOP_D_DD("_mm_cmpeq_pd", _mm_cmpeq_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmpeq_sd", _mm_cmpeq_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (dst_dbl[0] >= src2_dbl[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (dst_dbl[1] >= src2_dbl[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (dst_dbl[2] >= src2_dbl[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (dst_dbl[3] >= src2_dbl[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmp>=");
  BINARYOP_D_DD("_mm_cmpge_pd", _mm_cmpge_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmpge_sd", _mm_cmpge_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_cmpgt_epi16
  // _mm_cmpgt_epi32
  // _mm_cmpgt_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (dst_dbl[0] > src2_dbl[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (dst_dbl[1] > src2_dbl[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (dst_dbl[2] > src2_dbl[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (dst_dbl[3] > src2_dbl[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmp>");
  BINARYOP_D_DD("_mm_cmpgt_pd", _mm_cmpgt_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmpgt_sd", _mm_cmpgt_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (dst_dbl[0] <= src2_dbl[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (dst_dbl[1] <= src2_dbl[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (dst_dbl[2] <= src2_dbl[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (dst_dbl[3] <= src2_dbl[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmp<=");
  BINARYOP_D_DD("_mm_cmple_pd", _mm_cmple_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmple_sd", _mm_cmple_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_cmplt_epi16
  // _mm_cmplt_epi32
  // _mm_cmplt_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (dst_dbl[0] < src2_dbl[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (dst_dbl[1] < src2_dbl[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (dst_dbl[2] < src2_dbl[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (dst_dbl[3] < src2_dbl[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmp<");
  BINARYOP_D_DD("_mm_cmplt_pd", _mm_cmplt_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmplt_sd", _mm_cmplt_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));

  /*_mm_cmpneq_pd
  _mm_cmpneq_sd
  _mm_cmpnge_pd
  _mm_cmpnge_sd
  _mm_cmpngt_pd
  _mm_cmpngt_sd
  _mm_cmpnle_pd
  _mm_cmpnle_sd
  _mm_cmpnlt_pd
  _mm_cmpnlt_sd*/

  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (!Isnan(dst_dbl[0]) && !Isnan(src2_dbl[0])) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (!Isnan(dst_dbl[1]) && !Isnan(src2_dbl[1])) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (!Isnan(dst_dbl[2]) && !Isnan(src2_dbl[2])) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (!Isnan(dst_dbl[3]) && !Isnan(src2_dbl[3])) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmpord");
  BINARYOP_D_DD("_mm_cmpord_pd", _mm_cmpord_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmpord_sd", _mm_cmpord_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = (Isnan(dst_dbl[0]) || Isnan(src2_dbl[0])) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[1] = (Isnan(dst_dbl[1]) || Isnan(src2_dbl[1])) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[2] = (Isnan(dst_dbl[2]) || Isnan(src2_dbl[2])) ? ucastd(0xFFFFFFFFU) : 0.f; dst_dbl[3] = (Isnan(dst_dbl[3]) || Isnan(src2_dbl[3])) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst_dbl), "scalar cmpunord");
  BINARYOP_D_DD("_mm_cmpunord_pd", _mm_cmpunord_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_cmpunord_sd", _mm_cmpunord_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));

  SETCHART("max");
  // _mm_max_epi16
  // _mm_max_epu8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = Max(dst_dbl[0], src2_dbl[0]); dst_dbl[1] = Max(dst_dbl[1], src2_dbl[1]); dst_dbl[2] = Max(dst_dbl[2], src2_dbl[2]); dst_dbl[3] = Max(dst_dbl[3], src2_dbl[3]); } ENDSCALAR(checksum_dst(dst_dbl), "scalar max");
  BINARYOP_D_DD("_mm_max_pd", _mm_max_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_max_sd", _mm_max_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_min_epi16
  // _mm_min_epu8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = Min(dst_dbl[0], src2_dbl[0]); dst_dbl[1] = Min(dst_dbl[1], src2_dbl[1]); dst_dbl[2] = Min(dst_dbl[2], src2_dbl[2]); dst_dbl[3] = Min(dst_dbl[3], src2_dbl[3]); } ENDSCALAR(checksum_dst(dst_dbl), "scalar min");
  BINARYOP_D_DD("_mm_min_pd", _mm_min_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  BINARYOP_D_DD("_mm_min_sd", _mm_min_sd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));

  SETCHART("shuffle");
  // _mm_extract_epi16
  // _mm_insert_epi16
  // _mm_shuffle_epi32
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[3] = dst_dbl[1]; dst_dbl[2] = dst_dbl[0]; dst_dbl[1] = src2_dbl[3]; dst_dbl[0] = src2_dbl[2]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar shuffle");
//  BINARYOP_D_DD("_mm_shuffle_pd", _mm_shuffle_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  START();
    __m128 o0 = _mm_load_pd(src_dbl);
    __m128 o1 = _mm_load_pd(src2_dbl);
    for(int i = 0; i < N; i += 4)
      o0 = _mm_shuffle_pd(o0, o1, _MM_SHUFFLE2(1, 0));
    _mm_store_pd(dst_dbl, o0);
  END(checksum_dst(dst_dbl), "_mm_shuffle_pd");

  // _mm_shufflehi_epi16
  // _mm_shufflelo_epi16
  // _mm_unpackhi_epi16
  // _mm_unpackhi_epi32
  // _mm_unpackhi_epi64
  // _mm_unpackhi_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[0] = dst_dbl[2]; dst_dbl[1] = src2_dbl[2]; dst_dbl[2] = dst_dbl[3]; dst_dbl[3] = src2_dbl[3]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar unpackhi_pd");
  BINARYOP_D_DD("_mm_unpackhi_pd", _mm_unpackhi_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));
  // _mm_unpacklo_epi16
  // _mm_unpacklo_epi32
  // _mm_unpacklo_epi64
  // _mm_unpacklo_epi8
  START(); dst_dbl[0] = src_dbl[0]; dst_dbl[1] = src_dbl[1]; dst_dbl[2] = src_dbl[2]; dst_dbl[3] = src_dbl[3]; for(int i = 0; i < N; ++i) { dst_dbl[2] = dst_dbl[1]; dst_dbl[1] = dst_dbl[0]; dst_dbl[0] = src2_dbl[0]; dst_dbl[3] = src2_dbl[1]; } ENDSCALAR(checksum_dst(dst_dbl), "scalar unpacklo_pd");
  BINARYOP_D_DD("_mm_unpacklo_pd", _mm_unpacklo_pd, _mm_load_pd(src_dbl), _mm_load_pd(src2_dbl));

  // Benchmarks end:
  printf("]}\n");
}
