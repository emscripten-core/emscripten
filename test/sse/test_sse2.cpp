/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
// This file uses SSE2 by calling different functions with different interesting
// inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <emmintrin.h>
#define ENABLE_SSE2
#include "test_sse.h"

bool testNaNBits = true;

float *interesting_floats = get_interesting_floats();
int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
uint32_t *interesting_ints = get_interesting_ints();
int numInterestingInts = sizeof(interesting_ints_)/sizeof(interesting_ints_[0]);
double *interesting_doubles = get_interesting_doubles();
int numInterestingDoubles = sizeof(interesting_doubles_)/sizeof(interesting_doubles_[0]);

// SSE2 Arithmetic instructions:
void NOINLINE test_add_epi16() { M128i_M128i_M128i(_mm_add_epi16); }
void NOINLINE test_add_epi32() { M128i_M128i_M128i(_mm_add_epi32); }
void NOINLINE test_add_epi64() { M128i_M128i_M128i(_mm_add_epi64); }
void NOINLINE test_add_epi8() { M128i_M128i_M128i(_mm_add_epi8); }
  
void NOINLINE test_add_pd() { testNaNBits = false; Ret_M128d_M128d(__m128d, _mm_add_pd); }
void NOINLINE test_add_sd() { testNaNBits = false; Ret_M128d_M128d(__m128d, _mm_add_sd); }
  
void NOINLINE test_adds_epi16() { testNaNBits = true; M128i_M128i_M128i(_mm_adds_epi16); }
void NOINLINE test_adds_epi8() { testNaNBits = true; M128i_M128i_M128i(_mm_adds_epi8); }
void NOINLINE test_adds_epu16() { testNaNBits = true; M128i_M128i_M128i(_mm_adds_epu16); }
void NOINLINE test_adds_epu8() { testNaNBits = true; M128i_M128i_M128i(_mm_adds_epu8); }
void NOINLINE test_div_pd() { testNaNBits = true; Ret_M128d_M128d(__m128d, _mm_div_pd); }
void NOINLINE test_div_sd() { testNaNBits = true; Ret_M128d_M128d(__m128d, _mm_div_sd); }
void NOINLINE test_madd_epi16() { testNaNBits = true; M128i_M128i_M128i(_mm_madd_epi16); }
void NOINLINE test_mul_epu32() { testNaNBits = true; M128i_M128i_M128i(_mm_mul_epu32); }
  
void NOINLINE test_mul_pd() { testNaNBits = false; Ret_M128d_M128d(__m128d, _mm_mul_pd); }
void NOINLINE test_mul_sd() { testNaNBits = false; Ret_M128d_M128d(__m128d, _mm_mul_sd); }
  
void NOINLINE test_mulhi_epi16() { testNaNBits = true; M128i_M128i_M128i(_mm_mulhi_epi16); }
void NOINLINE test_mulhi_epu16() { testNaNBits = true; M128i_M128i_M128i(_mm_mulhi_epu16); }
void NOINLINE test_mullo_epi16() { testNaNBits = true; M128i_M128i_M128i(_mm_mullo_epi16); }
void NOINLINE test_sad_epu8() { testNaNBits = true; M128i_M128i_M128i(_mm_sad_epu8); }
void NOINLINE test_sub_epi16() { testNaNBits = true; M128i_M128i_M128i(_mm_sub_epi16); }
void NOINLINE test_sub_epi32() { testNaNBits = true; M128i_M128i_M128i(_mm_sub_epi32); }
void NOINLINE test_sub_epi64() { testNaNBits = true; M128i_M128i_M128i(_mm_sub_epi64); }
void NOINLINE test_sub_epi8() { testNaNBits = true; M128i_M128i_M128i(_mm_sub_epi8); }
void NOINLINE test_sub_pd() { testNaNBits = true; Ret_M128d_M128d(__m128d, _mm_sub_pd); }
void NOINLINE test_sub_sd() { testNaNBits = true; Ret_M128d_M128d(__m128d, _mm_sub_sd); }
void NOINLINE test_subs_epi16() { testNaNBits = true; M128i_M128i_M128i(_mm_subs_epi16); }
void NOINLINE test_subs_epi8() { testNaNBits = true; M128i_M128i_M128i(_mm_subs_epi8); }
void NOINLINE test_subs_epu16() { testNaNBits = true; M128i_M128i_M128i(_mm_subs_epu16); }
void NOINLINE test_subs_epu8() { testNaNBits = true; M128i_M128i_M128i(_mm_subs_epu8); }

// SSE2 Cast functions:
void NOINLINE test_castpd_ps() { Ret_M128d(__m128, _mm_castpd_ps); }
void NOINLINE test_castpd_si128() { Ret_M128d(__m128i, _mm_castpd_si128); }
void NOINLINE test_castps_pd() { Ret_M128(__m128d, _mm_castps_pd); }
void NOINLINE test_castps_si128() { Ret_M128(__m128i, _mm_castps_si128); }
void NOINLINE test_castsi128_pd() { Ret_M128i(__m128d, _mm_castsi128_pd); }
void NOINLINE test_castsi128_ps() { Ret_M128i(__m128, _mm_castsi128_ps); }

// SSE2 Compare instructions:
void NOINLINE test_cmpeq_epi16() { M128i_M128i_M128i(_mm_cmpeq_epi16); }
void NOINLINE test_cmpeq_epi32() { M128i_M128i_M128i(_mm_cmpeq_epi32); }
void NOINLINE test_cmpeq_epi8() { M128i_M128i_M128i(_mm_cmpeq_epi8); }
void NOINLINE test_cmpeq_pd() { Ret_M128d_M128d(__m128d, _mm_cmpeq_pd); }
void NOINLINE test_cmpeq_sd() { Ret_M128d_M128d(__m128d, _mm_cmpeq_sd); }
void NOINLINE test_cmpge_pd() { Ret_M128d_M128d(__m128d, _mm_cmpge_pd); }
void NOINLINE test_cmpge_sd() { Ret_M128d_M128d(__m128d, _mm_cmpge_sd); }
void NOINLINE test_cmpgt_epi16() { M128i_M128i_M128i(_mm_cmpgt_epi16); }
void NOINLINE test_cmpgt_epi32() { M128i_M128i_M128i(_mm_cmpgt_epi32); }
void NOINLINE test_cmpgt_epi8() { M128i_M128i_M128i(_mm_cmpgt_epi8); }
void NOINLINE test_cmpgt_pd() { Ret_M128d_M128d(__m128d, _mm_cmpgt_pd); }
void NOINLINE test_cmpgt_sd() { Ret_M128d_M128d(__m128d, _mm_cmpgt_sd); }
void NOINLINE test_cmple_pd() { Ret_M128d_M128d(__m128d, _mm_cmple_pd); }
void NOINLINE test_cmple_sd() { Ret_M128d_M128d(__m128d, _mm_cmple_sd); }
void NOINLINE test_cmplt_epi16() { M128i_M128i_M128i(_mm_cmplt_epi16); }
void NOINLINE test_cmplt_epi32() { M128i_M128i_M128i(_mm_cmplt_epi32); }
void NOINLINE test_cmplt_epi8() { M128i_M128i_M128i(_mm_cmplt_epi8); }
void NOINLINE test_cmplt_pd() { Ret_M128d_M128d(__m128d, _mm_cmplt_pd); }
void NOINLINE test_cmplt_sd() { Ret_M128d_M128d(__m128d, _mm_cmplt_sd); }
void NOINLINE test_cmpneq_pd() { Ret_M128d_M128d(__m128d, _mm_cmpneq_pd); }
void NOINLINE test_cmpneq_sd() { Ret_M128d_M128d(__m128d, _mm_cmpneq_sd); }
void NOINLINE test_cmpnge_pd() { Ret_M128d_M128d(__m128d, _mm_cmpnge_pd); }
void NOINLINE test_cmpnge_sd() { Ret_M128d_M128d(__m128d, _mm_cmpnge_sd); }
void NOINLINE test_cmpngt_pd() { Ret_M128d_M128d(__m128d, _mm_cmpngt_pd); }
void NOINLINE test_cmpngt_sd() { Ret_M128d_M128d(__m128d, _mm_cmpngt_sd); }
void NOINLINE test_cmpnle_pd() { Ret_M128d_M128d(__m128d, _mm_cmpnle_pd); }
void NOINLINE test_cmpnle_sd() { Ret_M128d_M128d(__m128d, _mm_cmpnle_sd); }
void NOINLINE test_cmpnlt_pd() { Ret_M128d_M128d(__m128d, _mm_cmpnlt_pd); }
void NOINLINE test_cmpnlt_sd() { Ret_M128d_M128d(__m128d, _mm_cmpnlt_sd); }
void NOINLINE test_cmpord_pd() { Ret_M128d_M128d(__m128d, _mm_cmpord_pd); }
void NOINLINE test_cmpord_sd() { Ret_M128d_M128d(__m128d, _mm_cmpord_sd); }
void NOINLINE test_cmpunord_pd() { Ret_M128d_M128d(__m128d, _mm_cmpunord_pd); }
void NOINLINE test_cmpunord_sd() { Ret_M128d_M128d(__m128d, _mm_cmpunord_sd); }
void NOINLINE test_comieq_sd() { Ret_M128d_M128d(int, _mm_comieq_sd); }
void NOINLINE test_comige_sd() { Ret_M128d_M128d(int, _mm_comige_sd); }
void NOINLINE test_comigt_sd() { Ret_M128d_M128d(int, _mm_comigt_sd); }
void NOINLINE test_comile_sd() { Ret_M128d_M128d(int, _mm_comile_sd); }
void NOINLINE test_comilt_sd() { Ret_M128d_M128d(int, _mm_comilt_sd); }
void NOINLINE test_comineq_sd() { Ret_M128d_M128d(int, _mm_comineq_sd); }
void NOINLINE test_ucomieq_sd() { Ret_M128d_M128d(int, _mm_ucomieq_sd); }
void NOINLINE test_ucomige_sd() { Ret_M128d_M128d(int, _mm_ucomige_sd); }
void NOINLINE test_ucomigt_sd() { Ret_M128d_M128d(int, _mm_ucomigt_sd); }
void NOINLINE test_ucomile_sd() { Ret_M128d_M128d(int, _mm_ucomile_sd); }
void NOINLINE test_ucomilt_sd() { Ret_M128d_M128d(int, _mm_ucomilt_sd); }
void NOINLINE test_ucomineq_sd() { Ret_M128d_M128d(int, _mm_ucomineq_sd); }

// SSE2 Convert instructions:
void NOINLINE test_cvtepi32_pd() { Ret_M128i(__m128d, _mm_cvtepi32_pd); }
void NOINLINE test_cvtepi32_ps() { Ret_M128i(__m128, _mm_cvtepi32_ps); }
void NOINLINE test_cvtpd_epi32() { Ret_M128d(__m128i, _mm_cvtpd_epi32); }
void NOINLINE test_cvtpd_ps() { Ret_M128d(__m128, _mm_cvtpd_ps); }
void NOINLINE test_cvtps_epi32() { Ret_M128(__m128i, _mm_cvtps_epi32); }
void NOINLINE test_cvtps_pd() { Ret_M128(__m128d,  _mm_cvtps_pd); }
void NOINLINE test_cvtsd_f64() { Ret_M128d(double, _mm_cvtsd_f64); }
void NOINLINE test_cvtsd_si32() { Ret_M128d(int, _mm_cvtsd_si32); }
void NOINLINE test_cvtsd_si64() { Ret_M128d(int64_t, _mm_cvtsd_si64); } // _mm_cvtsd_si64x is an alias to this.
void NOINLINE test_cvtsi128_si32() { Ret_M128i(int, _mm_cvtsi128_si32); }
void NOINLINE test_cvtsi128_si64() { Ret_M128i(int64_t, _mm_cvtsi128_si64); } // _mm_cvtsi128_si64x is an alias to this.
void NOINLINE test_cvtsi32_sd() { Ret_M128d_int(__m128d, _mm_cvtsi32_sd); }
void NOINLINE test_cvtsi32_si128() { Ret_int(__m128i, _mm_cvtsi32_si128); }
void NOINLINE test_cvtsi64_sd() { Ret_M128d_int64(__m128d, _mm_cvtsi64_sd); } // _mm_cvtsi64x_sd is an alias to this.
void NOINLINE test_cvtsi64_si128() { Ret_int64(__m128i, _mm_cvtsi64_si128); } // _mm_cvtsi64x_si128 is an alias to this.
void NOINLINE test_cvtss_sd() { Ret_M128d_M128(__m128d, _mm_cvtss_sd); }
void NOINLINE test_cvttpd_epi32() { Ret_M128d(__m128i, _mm_cvttpd_epi32); }
void NOINLINE test_cvttps_epi32() { Ret_M128(__m128i, _mm_cvttps_epi32); }
void NOINLINE test_cvttsd_si32() { Ret_M128d(int, _mm_cvttsd_si32); }
void NOINLINE test_cvttsd_si64() { Ret_M128d(int64_t, _mm_cvttsd_si64); } // _mm_cvttsd_si64x is an alias to this.

// SSE2 Elementary Math Functions instructions:
void NOINLINE test_sqrt_pd() { Ret_M128d(__m128d, _mm_sqrt_pd); }
void NOINLINE test_sqrt_sd() { Ret_M128d_M128d(__m128d, _mm_sqrt_sd); }

// SSE2 General Support instructions:
void NOINLINE test_clflush() { _mm_clflush(interesting_floats); }
void NOINLINE test_lfence() { _mm_lfence(); }
void NOINLINE test_mfence() { _mm_mfence(); }
void NOINLINE test_pause() { _mm_pause(); }

// SSE2 Load functions:
void NOINLINE test_load_pd() { Ret_DoublePtr(__m128d, _mm_load_pd, 2, 2); }
void NOINLINE test_load_pd1() { Ret_DoublePtr(__m128d, _mm_load_pd1, 1, 1); }
void NOINLINE test_load_sd() { Ret_DoublePtr(__m128d, _mm_load_sd, 1, 1); }
void NOINLINE test_load_si128() { Ret_IntPtr(__m128i, _mm_load_si128, __m128i*, 4, 4); }
void NOINLINE test_load1_pd() { Ret_DoublePtr(__m128d, _mm_load1_pd, 1, 1); }
void NOINLINE test_loadh_pd() { Ret_M128d_DoublePtr(__m128d, _mm_loadh_pd, double*, 1, 1); }
void NOINLINE test_loadl_epi64() { Ret_IntPtr(__m128i, _mm_loadl_epi64, __m128i*, 2, 1); }
void NOINLINE test_loadl_pd() { Ret_M128d_DoublePtr(__m128d, _mm_loadl_pd, double*, 1, 1); }
void NOINLINE test_loadr_pd() { Ret_DoublePtr(__m128d, _mm_loadr_pd, 2, 2); }
void NOINLINE test_loadu_pd() { Ret_DoublePtr(__m128d, _mm_loadu_pd, 2, 1); }
void NOINLINE test_loadu_si128() { Ret_IntPtr(__m128i, _mm_loadu_si128, __m128i*, 4, 1); }
void NOINLINE test_loadu_si32() { Ret_IntPtr(__m128i, _mm_loadu_si32, __m128i*, 1, 1); }

// SSE2 Logical instructions:
void NOINLINE test_and_pd() { Ret_M128d_M128d(__m128d, _mm_and_pd); }
void NOINLINE test_and_si128() { M128i_M128i_M128i(_mm_and_si128); }
void NOINLINE test_andnot_pd() { Ret_M128d_M128d(__m128d, _mm_andnot_pd); }
void NOINLINE test_andnot_si128() { M128i_M128i_M128i(_mm_andnot_si128); }
void NOINLINE test_or_pd() { Ret_M128d_M128d(__m128d, _mm_or_pd); }
void NOINLINE test_or_si128() { M128i_M128i_M128i(_mm_or_si128); }
void NOINLINE test_xor_pd() { Ret_M128d_M128d(__m128d, _mm_xor_pd); }
void NOINLINE test_xor_si128() { M128i_M128i_M128i(_mm_xor_si128); }

// SSE2 Miscellaneous instructions:
void NOINLINE test_movemask_epi8() { Ret_M128i(int, _mm_movemask_epi8); }
void NOINLINE test_movemask_pd() { Ret_M128d(int, _mm_movemask_pd); }
void NOINLINE test_packs_epi16() { M128i_M128i_M128i(_mm_packs_epi16); }
void NOINLINE test_packs_epi32() { M128i_M128i_M128i(_mm_packs_epi32); }
void NOINLINE test_packus_epi16() { M128i_M128i_M128i(_mm_packus_epi16); }

// SSE2 Move instructions:
void NOINLINE test_move_epi64() { Ret_M128i(__m128i, _mm_move_epi64); }
void NOINLINE test_move_sd() { Ret_M128d_M128d(__m128d, _mm_move_sd); }

// SSE2 Probability/Statistics instructions:
void NOINLINE test_avg_epu16() { M128i_M128i_M128i(_mm_avg_epu16); }
void NOINLINE test_avg_epu8() { M128i_M128i_M128i(_mm_avg_epu8); }

// SSE2 Shift instructions:
void NOINLINE test_sll_epi16() { M128i_M128i_M128i(_mm_sll_epi16); }
void NOINLINE test_sll_epi32() { M128i_M128i_M128i(_mm_sll_epi32); }
void NOINLINE test_sll_epi64() { M128i_M128i_M128i(_mm_sll_epi64); }
void NOINLINE test_slli_epi16() { Ret_M128i_Tint(__m128i, _mm_slli_epi16); }
void NOINLINE test_slli_epi32() { Ret_M128i_Tint(__m128i, _mm_slli_epi32); }
void NOINLINE test_slli_epi64() { Ret_M128i_Tint(__m128i, _mm_slli_epi64); }
void NOINLINE test_slli_si128() { Ret_M128i_Tint(__m128i, _mm_slli_si128); } // _mm_bslli_si128 is an alias to this.
void NOINLINE test_sra_epi16() { M128i_M128i_M128i(_mm_sra_epi16); }
void NOINLINE test_sra_epi32() { M128i_M128i_M128i(_mm_sra_epi32); }
void NOINLINE test_srai_epi16() { Ret_M128i_Tint(__m128i, _mm_srai_epi16); }
void NOINLINE test_srai_epi32() { Ret_M128i_Tint(__m128i, _mm_srai_epi32); }
void NOINLINE test_srl_epi16() { M128i_M128i_M128i(_mm_srl_epi16); }
void NOINLINE test_srl_epi32() { M128i_M128i_M128i(_mm_srl_epi32); }
void NOINLINE test_srl_epi64() { M128i_M128i_M128i(_mm_srl_epi64); }
void NOINLINE test_srli_epi16() { Ret_M128i_Tint(__m128i, _mm_srli_epi16); }
void NOINLINE test_srli_epi32() { Ret_M128i_Tint(__m128i, _mm_srli_epi32); }
void NOINLINE test_srli_epi64() { Ret_M128i_Tint(__m128i, _mm_srli_epi64); }
void NOINLINE test_srli_si128() { Ret_M128i_Tint(__m128i, _mm_srli_si128); } // _mm_bsrli_si128 is an alias to this.

// SSE2 Special Math instructions:
void NOINLINE test_max_epi16() { M128i_M128i_M128i(_mm_max_epi16); }
void NOINLINE test_max_epu8() { M128i_M128i_M128i(_mm_max_epu8); }
void NOINLINE test_max_pd() { Ret_M128d_M128d(__m128d, _mm_max_pd); }
void NOINLINE test_max_sd() { Ret_M128d_M128d(__m128d, _mm_max_sd); }
void NOINLINE test_min_epi16() { M128i_M128i_M128i(_mm_min_epi16); }
void NOINLINE test_min_epu8() { M128i_M128i_M128i(_mm_min_epu8); }
void NOINLINE test_min_pd() { Ret_M128d_M128d(__m128d, _mm_min_pd); }
void NOINLINE test_min_sd() { Ret_M128d_M128d(__m128d, _mm_min_sd); }

// SSE2 Store instructions:
void NOINLINE test_maskmoveu_si128() { void_M128i_M128i_OutIntPtr(_mm_maskmoveu_si128, char*, 16, 1); }
void NOINLINE test_store_pd() { void_OutDoublePtr_M128d(_mm_store_pd, double*, 16, 16); }
void NOINLINE test_store_sd() { void_OutDoublePtr_M128d(_mm_store_sd, double*, 8, 1); }
void NOINLINE test_store_si128() { void_OutIntPtr_M128i(_mm_store_si128, __m128i*, 16, 16); }
void NOINLINE test_store1_pd() { void_OutDoublePtr_M128d(_mm_store1_pd, double*, 16, 16); } // _mm_store_pd1 is an alias to this.
void NOINLINE test_storeh_pd() { void_OutDoublePtr_M128d(_mm_storeh_pd, double*, 8, 1); }
void NOINLINE test_storel_epi64() { void_OutIntPtr_M128i(_mm_storel_epi64, __m128i*, 8, 1); }
void NOINLINE test_storel_pd() { void_OutDoublePtr_M128d(_mm_storel_pd, double*, 8, 1); }
void NOINLINE test_storer_pd() { void_OutDoublePtr_M128d(_mm_storer_pd, double*, 16, 16); }
void NOINLINE test_storeu_pd() { void_OutDoublePtr_M128d(_mm_storeu_pd, double*, 16, 1); }
void NOINLINE test_storeu_si16() { void_OutIntPtr_M128i(_mm_storeu_si16, unsigned short*, 2, 1); }
void NOINLINE test_storeu_si32() { void_OutIntPtr_M128i(_mm_storeu_si32, __m128i*, 4, 1); }
void NOINLINE test_storeu_si64() { void_OutIntPtr_M128i(_mm_storeu_si64, __m64*, 8, 1); }
void NOINLINE test_storeu_si128() { void_OutIntPtr_M128i(_mm_storeu_si128, __m128i*, 16, 1); }
void NOINLINE test_stream_pd() { void_OutDoublePtr_M128d(_mm_stream_pd, double*, 16, 16); }
void NOINLINE test_stream_si128() { void_OutIntPtr_M128i(_mm_stream_si128, __m128i*, 16, 16); }
void NOINLINE test_stream_si32() { void_OutIntPtr_int(_mm_stream_si32, int*, 4, 1); }
void NOINLINE test_stream_si64() { void_OutIntPtr_int64(_mm_stream_si64, long long*, 8, 1); }

// SSE2 Swizzle instructions:
void NOINLINE test_extract_epi16() { Ret_M128i_Tint(int, _mm_extract_epi16); }
void NOINLINE test_mm_insert_epi16() { Ret_M128i_int_Tint(__m128i, _mm_insert_epi16); }
void NOINLINE test_shuffle_epi32() { Ret_M128i_Tint(__m128i, _mm_shuffle_epi32); }
void NOINLINE test_shuffle_pd() { Ret_M128d_M128d_Tint(__m128d, _mm_shuffle_pd); }
void NOINLINE test_shufflehi_epi16() { Ret_M128i_Tint(__m128i, _mm_shufflehi_epi16); }
void NOINLINE test_shufflelo_epi16() { Ret_M128i_Tint(__m128i, _mm_shufflelo_epi16); }
void NOINLINE test_unpackhi_epi16() { M128i_M128i_M128i(_mm_unpackhi_epi16); }
void NOINLINE test_unpackhi_epi32() { M128i_M128i_M128i(_mm_unpackhi_epi32); }
void NOINLINE test_unpackhi_epi64() { M128i_M128i_M128i(_mm_unpackhi_epi64); }
void NOINLINE test_unpackhi_epi8() { M128i_M128i_M128i(_mm_unpackhi_epi8); }
void NOINLINE test_unpackhi_pd() { Ret_M128d_M128d(__m128d, _mm_unpackhi_pd); }
void NOINLINE test_unpacklo_epi16() { M128i_M128i_M128i(_mm_unpacklo_epi16); }
void NOINLINE test_unpacklo_epi32() { M128i_M128i_M128i(_mm_unpacklo_epi32); }
void NOINLINE test_unpacklo_epi64() { M128i_M128i_M128i(_mm_unpacklo_epi64); }
void NOINLINE test_unpacklo_epi8() { M128i_M128i_M128i(_mm_unpacklo_epi8); }
void NOINLINE test_unpacklo_pd() { Ret_M128d_M128d(__m128d, _mm_unpacklo_pd); }

int main() {
  assert(numInterestingFloats % 4 == 0);
  assert(numInterestingInts % 4 == 0);
  assert(numInterestingDoubles % 4 == 0);

  test_add_epi16();
  test_add_epi32();
  test_add_epi64();
  test_add_epi8();
  test_add_pd();
  test_add_sd();
  test_adds_epi16();
  test_adds_epi8();
  test_adds_epu16();
  test_adds_epu8();
  test_div_pd();
  test_div_sd();
  test_madd_epi16();
  test_mul_epu32();
  test_mul_pd();
  test_mul_sd();
  test_mulhi_epi16();
  test_mulhi_epu16();
  test_mullo_epi16();
  test_sad_epu8();
  test_sub_epi16();
  test_sub_epi32();
  test_sub_epi64();
  test_sub_epi8();
  test_sub_pd();
  test_sub_sd();
  test_subs_epi16();
  test_subs_epi8();
  test_subs_epu16();
  test_subs_epu8();

  test_castpd_ps();
  test_castpd_si128();
  test_castps_pd();
  test_castps_si128();
  test_castsi128_pd();
  test_castsi128_ps();

  test_cmpeq_epi16();
  test_cmpeq_epi32();
  test_cmpeq_epi8();
  test_cmpeq_pd();
  test_cmpeq_sd();
  test_cmpge_pd();
  test_cmpge_sd();
  test_cmpgt_epi16();
  test_cmpgt_epi32();
  test_cmpgt_epi8();
  test_cmpgt_pd();
  test_cmpgt_sd();
  test_cmple_pd();
  test_cmple_sd();
  test_cmplt_epi16();
  test_cmplt_epi32();
  test_cmplt_epi8();
  test_cmplt_pd();
  test_cmplt_sd();
  test_cmpneq_pd();
  test_cmpneq_sd();
  test_cmpnge_pd();
  test_cmpnge_sd();
  test_cmpngt_pd();
  test_cmpngt_sd();
  test_cmpnle_pd();
  test_cmpnle_sd();
  test_cmpnlt_pd();
  test_cmpnlt_sd();
  test_cmpord_pd();
  test_cmpord_sd();
  test_cmpunord_pd();
  test_cmpunord_sd();
  test_comieq_sd();
  test_comige_sd();
  test_comigt_sd();
  test_comile_sd();
  test_comilt_sd();
  test_comineq_sd();
  test_ucomieq_sd();
  test_ucomige_sd();
  test_ucomigt_sd();
  test_ucomile_sd();
  test_ucomilt_sd();
  test_ucomineq_sd();

  test_cvtepi32_pd();
  test_cvtepi32_ps();
  test_cvtpd_epi32();
  test_cvtpd_ps();
  test_cvtps_epi32();
  test_cvtps_pd();
  test_cvtsd_f64();
  test_cvtsd_si32();
  test_cvtsd_si64();
  test_cvtsi128_si32();
  test_cvtsi128_si64();
  test_cvtsi32_sd();
  test_cvtsi32_si128();
  test_cvtsi64_sd();
  test_cvtsi64_si128();
  test_cvtss_sd();
  test_cvttpd_epi32();
  test_cvttps_epi32();
  test_cvttsd_si32();
  test_cvttsd_si64();

  test_sqrt_pd();
  test_sqrt_sd();

  test_clflush();
  test_lfence();
  test_mfence();
  test_pause();

  test_load_pd();
  test_load_pd1();
  test_load_sd();
  test_load_si128();
  test_load1_pd();
  test_loadh_pd();
  test_loadl_epi64();
  test_loadl_pd();
  test_loadr_pd();
  test_loadu_pd();
  test_loadu_si128();
  test_loadu_si32();

  test_and_pd();
  test_and_si128();
  test_andnot_pd();
  test_andnot_si128();
  test_or_pd();
  test_or_si128();
  test_xor_pd();
  test_xor_si128();

  test_movemask_epi8();
  test_movemask_pd();
  test_packs_epi16();
  test_packs_epi32();
  test_packus_epi16();

  test_move_epi64();
  test_move_sd();

  test_avg_epu16();
  test_avg_epu8();

/*
  // TODO: SSE2 Set functions:
  _mm_set_epi16
  _mm_set_epi32
  _mm_set_epi64
  _mm_set_epi64x
  _mm_set_epi8
  _mm_set_pd
  _mm_set_pd1
  _mm_set_sd
  _mm_set1_epi16
  _mm_set1_epi32
  _mm_set1_epi64
  _mm_set1_epi64x
  _mm_set1_epi8
  _mm_set1_pd
  _mm_setr_epi16
  _mm_setr_epi32
  _mm_setr_epi64
  _mm_setr_epi8
  _mm_setr_pd
  _mm_setzero_pd
  _mm_setzero_si128
*/

  test_sll_epi16();
  test_sll_epi32();
  test_sll_epi64();
  test_slli_epi16();
  test_slli_epi32();
  test_slli_epi64();
  test_slli_si128();
  test_sra_epi16();
  test_sra_epi32();
  test_srai_epi16();
  test_srai_epi32();
  test_srl_epi16();
  test_srl_epi32();
  test_srl_epi64();
  test_srli_epi16();
  test_srli_epi32();
  test_srli_epi64();
  test_srli_si128();

  test_max_epi16();
  test_max_epu8();
  test_max_pd();
  test_max_sd();
  test_min_epi16();
  test_min_epu8();
  test_min_pd();
  test_min_sd();

  test_maskmoveu_si128();
  test_store_pd();
  test_store_sd();
  test_store_si128();
  test_store1_pd();
  test_storeh_pd();
  test_storel_epi64();
  test_storel_pd();
  test_storer_pd();
  test_storeu_pd();
  test_storeu_si16();
  test_storeu_si32();
  test_storeu_si64();
  test_storeu_si128();
  test_stream_pd();
  test_stream_si128();
  test_stream_si32();
  test_stream_si64();

  test_extract_epi16();
  test_mm_insert_epi16();
  test_shuffle_epi32();
  test_shuffle_pd();
  test_shufflehi_epi16();
  test_shufflelo_epi16();
  test_unpackhi_epi16();
  test_unpackhi_epi32();
  test_unpackhi_epi64();
  test_unpackhi_epi8();
  test_unpackhi_pd();
  test_unpacklo_epi16();
  test_unpacklo_epi32();
  test_unpacklo_epi64();
  test_unpacklo_epi8();
  test_unpacklo_pd();

#ifdef __EMSCRIPTEN__
  _mm_undefined_si128();
  _mm_undefined_pd();
#endif
}
