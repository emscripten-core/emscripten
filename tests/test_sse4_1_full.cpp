// This file uses SSE4.1 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <smmintrin.h>
#define ENABLE_SSE2
#include "test_sse_full.h"

float *interesting_floats = get_interesting_floats();
int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
uint32_t *interesting_ints = get_interesting_ints();
int numInterestingInts = sizeof(interesting_ints_)/sizeof(interesting_ints_[0]);
double *interesting_doubles = get_interesting_doubles();
int numInterestingDoubles = sizeof(interesting_doubles_)/sizeof(interesting_doubles_[0]);

int main()
{
	assert(numInterestingFloats % 4 == 0);
	assert(numInterestingInts % 4 == 0);
	assert(numInterestingDoubles % 4 == 0);	

	Ret_M128i_M128i_Tint(__m128i, _mm_blend_epi16);
	Ret_M128d_M128d_Tint(__m128d, _mm_blend_pd);
	Ret_M128_M128_Tint(__m128, _mm_blend_ps);
	// _mm_blendv_epi8
	// _mm_blendv_pd
	// _mm_blendv_ps
	// _mm_ceil_pd
	// _mm_ceil_ps
	// _mm_ceil_sd
	// _mm_ceil_ss
	// M128i_M128i_M128i(_mm_cmpeq_epi64);
	// Ret_M128i(__m128i, _mm_cvtepi16_epi32);
	// Ret_M128i(__m128i, _mm_cvtepi16_epi64);
	// Ret_M128i(__m128i, _mm_cvtepi32_epi64);
	// Ret_M128i(__m128i, _mm_cvtepi8_epi16);
	// Ret_M128i(__m128i, _mm_cvtepi8_epi32);
	// Ret_M128i(__m128i, _mm_cvtepi8_epi64);
	// _mm_cvtepu16_epi32
	// _mm_cvtepu16_epi64
	// _mm_cvtepu32_epi64
	// _mm_cvtepu8_epi16
	// _mm_cvtepu8_epi32
	// _mm_cvtepu8_epi64
	// _mm_dp_pd
	// _mm_dp_ps
	Ret_M128i_Tint(int, _mm_extract_epi32);
	Ret_M128i_Tint(int, _mm_extract_epi8);
	// Ret_M128i_Tint(long long, _mm_extract_epi64);
	Ret_M128i_Tint(int, _mm_extract_epi8);
	Ret_M128_Tint(float, _mm_extract_ps);
	// _mm_floor_pd
	// _mm_floor_ps
	// _mm_floor_sd
	// _mm_floor_ss
	// _mm_insert_epi32
	// _mm_insert_epi64
	// _mm_insert_epi8
	// _mm_insert_ps
	M128i_M128i_M128i(_mm_max_epi32);
	M128i_M128i_M128i(_mm_max_epi8);
	M128i_M128i_M128i(_mm_max_epu16);
	M128i_M128i_M128i(_mm_max_epu32);
	M128i_M128i_M128i(_mm_min_epi32);
	M128i_M128i_M128i(_mm_min_epi8);
	M128i_M128i_M128i(_mm_min_epu16);
	M128i_M128i_M128i(_mm_min_epu32);
	// _mm_minpos_epu16
	// _mm_mpsadbw_epu8
	// M128i_M128i_M128i(_mm_mul_epi32);
	M128i_M128i_M128i(_mm_mullo_epi32);
	// _mm_packus_epi32
	// _mm_round_pd
	// _mm_round_ps
	// _mm_round_sd
	// _mm_round_ss
	// _mm_stream_load_si128
	// _mm_test_all_ones
	// _mm_test_all_zeros
	// _mm_test_mix_ones_zeros
	// _mm_testc_si128
	// _mm_testnzc_si128
	// _mm_testz_si128

	// SSE 4.2:
	// _mm_cmpestra
	// _mm_cmpestrc
	// _mm_cmpestri
	// _mm_cmpestrm
	// _mm_cmpestro
	// _mm_cmpestrs
	// _mm_cmpestrz
	// M128i_M128i_M128i(_mm_cmpgt_epi64);
	// _mm_cmpistra
	// _mm_cmpistrc
	// _mm_cmpistri
	// _mm_cmpistrm
	// _mm_cmpistro
	// _mm_cmpistrs
	// _mm_cmpistrz
	// _mm_crc32_u16
	// _mm_crc32_u32
	// _mm_crc32_u64
	// _mm_crc32_u8
}
