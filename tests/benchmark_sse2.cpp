#include <emmintrin.h>
#include "benchmark_sse.h"

int main()
{
#ifndef __EMSCRIPTEN__
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif

	printf ("{ \"workload\": %u, \"results\": [\n", N);
	assert(N%2 == 0); // Don't care about the tail for now.
	double *src = get_src_d();//(float*)aligned_alloc(16, N*sizeof(float));
	for(int i = 0; i < N; ++i)
		src[i] = (double)rand() / RAND_MAX;
	double *src2 = get_src2_d();//(float*)aligned_alloc(16, N*sizeof(float));
	for(int i = 0; i < N; ++i)
		src2[i] = (double)rand() / RAND_MAX;
	double *dst = get_dst_d();//(float*)aligned_alloc(16, N*sizeof(float));

	float scalarTime;
	SETCHART("load");
	START();
		for(int i = 0; i < N; ++i)
			dst[i] = src[i];
	ENDSCALAR(checksum_dst(dst), "scalar");

	LS_TEST("_mm_load_pd", _mm_load_pd, 0, _mm_store_pd, double*, 0, 2);
	LS_TEST("_mm_load_pd1", _mm_load_pd1, 1, _mm_store_pd, double*, 0, 2);
	LS_TEST("_mm_load_sd", _mm_load_sd, 1, _mm_store_pd, double*, 0, 2);
	// _mm_load_si128
	LS_TEST("_mm_load1_pd", _mm_load1_pd, 1, _mm_store_pd, double*, 0, 2);

	__m128d tempReg = _mm_set_pd(1.0, 2.0);
	LSH_TEST("_mm_loadh_pd", tempReg, _mm_loadh_pd, double*, 1, _mm_store_pd, double*, 0, 2);
	// _mm_loadl_epi64
	LSH_TEST("_mm_loadl_pd", tempReg, _mm_loadh_pd, double*, 1, _mm_store_pd, double*, 0, 2);

	LS_TEST("_mm_loadr_pd", _mm_loadr_pd, 0, _mm_store_pd, double*, 0, 2);
	LS_TEST("_mm_loadu_pd", _mm_loadu_pd, 1, _mm_store_pd, double*, 0, 2);
	// _mm_loadu_si128

	SETCHART("set");
/*	_mm_set_epi16
	_mm_set_epi32
	_mm_set_epi64
	_mm_set_epi64x
	_mm_set_epi8 */
	SS_TEST_D("_mm_set_pd", _mm_set_pd(src[i+2], src[i+0]));
	//SS_TEST_D("_mm_set_pd1", _mm_set_pd1(src[i]));
	SS_TEST_D("_mm_set_sd", _mm_set_sd(src[i]));
/*	_mm_set1_epi16
	_mm_set1_epi32
	_mm_set1_epi64
	_mm_set1_epi64x
	_mm_set1_epi8 */
	SS_TEST_D("_mm_set1_pd", _mm_set1_pd(src[i]));
/* 	_mm_setr_epi16
	_mm_setr_epi32
	_mm_setr_epi64
	_mm_setr_epi8 */
	SS_TEST_D("_mm_setr_pd", _mm_set_pd(src[i+2], src[i+0]));
	SS_TEST_D("_mm_setzero_pd", _mm_setzero_pd());
//	_mm_setzero_si128

	SETCHART("move");
	// _mm_move_epi64
	SS_TEST_D("_mm_move_sd", _mm_move_sd(_mm_load_pd(src+i), _mm_load_pd(src2+i)));

	SETCHART("store");
	// _mm_maskmoveu_si128
	LS_TEST("_mm_store_pd", _mm_load_pd, 0, _mm_store_pd, double*, 0, 2);
//	LS_TEST("_mm_store_pd1", _mm_load_pd, 0, _mm_store_pd1, double*, 0);
	LS_TEST("_mm_store_sd", _mm_load_pd, 0, _mm_store_sd, double*, 1, 2);
	// _mm_store_si128
	// _mm_store1_pd
	LS64_TEST("_mm_storeh_pi", _mm_load_pd, 0, _mm_storeh_pi, 1, 2);
	// _mm_storel_epi64
	LS64_TEST("_mm_storel_pi", _mm_load_pd, 0, _mm_storel_pi, 1, 2);
	LS_TEST("_mm_storer_pd", _mm_load_pd, 0, _mm_storer_pd, double*, 0, 2);
	LS_TEST("_mm_storeu_pd", _mm_load_pd, 0, _mm_storeu_pd, double*, 1, 2);
	// _mm_storeu_si128
	LS_TEST("_mm_stream_pd", _mm_load_pd, 0, _mm_stream_pd, double*, 0, 2);
	// _mm_stream_si128
	// _mm_stream_si32
	// _mm_stream_si64

	SETCHART("arithmetic");
	// _mm_add_epi16
	// _mm_add_epi32
	// _mm_add_epi64
	// _mm_add_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] += src2[0]; dst[1] += src2[1]; dst[2] += src2[2]; dst[3] += src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar add");
	BINARYOP_TEST_D("_mm_add_pd", _mm_add_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_add_sd", _mm_add_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_adds_epi16
	// _mm_adds_epi8
	// _mm_adds_epu16
	// _mm_adds_epu8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] /= src2[0]; dst[1] /= src2[1]; dst[2] /= src2[2]; dst[3] /= src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar div");
	BINARYOP_TEST_D("_mm_div_pd", _mm_div_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_div_sd", _mm_div_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_madd_epi16
	// _mm_mul_epu32
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] *= src2[0]; dst[1] *= src2[1]; dst[2] *= src2[2]; dst[3] *= src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar mul");
	BINARYOP_TEST_D("_mm_mul_pd", _mm_mul_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_mul_sd", _mm_mul_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_mulhi_epi16
	// _mm_mulhi_epu16
	// _mm_mullo_epi16
	// _mm_sad_epu8
	// _mm_sub_epi16
	// _mm_sub_epi32
	// _mm_sub_epi64
	// _mm_sub_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] -= src2[0]; dst[1] -= src2[1]; dst[2] -= src2[2]; dst[3] -= src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar sub");
	BINARYOP_TEST_D("_mm_sub_pd", _mm_sub_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_sub_sd", _mm_sub_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_subs_epi16
	// _mm_subs_epi8
	// _mm_subs_epu16
	// _mm_subs_epu8

	SETCHART("roots");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = sqrt(dst[0]); dst[1] = sqrt(dst[1]); dst[2] = sqrt(dst[2]); dst[3] = sqrt(dst[3]); } ENDSCALAR(checksum_dst(dst), "scalar sqrt");
	UNARYOP_TEST_D("_mm_sqrt_pd", _mm_sqrt_pd, _mm_load_pd(src));
//	UNARYOP_TEST_D("_mm_sqrt_sd", _mm_sqrt_sd, _mm_load_pd(src));

	SETCHART("logical");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastd(dcastu(dst[0]) & dcastu(src2[0])); dst[1] = ucastd(dcastu(dst[1]) & dcastu(src2[1])); dst[2] = ucastd(dcastu(dst[2]) & dcastu(src2[2])); dst[3] = ucastd(dcastu(dst[3]) & dcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar and");
	BINARYOP_TEST_D("_mm_and_pd", _mm_and_pd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_and_si128
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastd((~dcastu(dst[0])) & dcastu(src2[0])); dst[1] = ucastd((~dcastu(dst[1])) & dcastu(src2[1])); dst[2] = ucastd((~dcastu(dst[2])) & dcastu(src2[2])); dst[3] = ucastd((~dcastu(dst[3])) & dcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar andnot");
	BINARYOP_TEST_D("_mm_andnot_pd", _mm_andnot_pd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_andnot_si128
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastd(dcastu(dst[0]) | dcastu(src2[0])); dst[1] = ucastd(dcastu(dst[1]) | dcastu(src2[1])); dst[2] = ucastd(dcastu(dst[2]) | dcastu(src2[2])); dst[3] = ucastd(dcastu(dst[3]) | dcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar or");
	BINARYOP_TEST_D("_mm_or_pd", _mm_or_pd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_or_si128
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastd(dcastu(dst[0]) ^ dcastu(src2[0])); dst[1] = ucastd(dcastu(dst[1]) ^ dcastu(src2[1])); dst[2] = ucastd(dcastu(dst[2]) ^ dcastu(src2[2])); dst[3] = ucastd(dcastu(dst[3]) ^ dcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar xor");
	BINARYOP_TEST_D("_mm_xor_pd", _mm_xor_pd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_xor_si128

	SETCHART("cmp");
	// _mm_cmpeq_epi16
	// _mm_cmpeq_epi32
	// _mm_cmpeq_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] == src2[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] == src2[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] == src2[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] == src2[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp==");
	BINARYOP_TEST_D("_mm_cmpeq_pd", _mm_cmpeq_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmpeq_sd", _mm_cmpeq_sd, _mm_load_pd(src), _mm_load_pd(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] >= src2[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] >= src2[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] >= src2[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] >= src2[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp>=");
	BINARYOP_TEST_D("_mm_cmpge_pd", _mm_cmpge_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmpge_sd", _mm_cmpge_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_cmpgt_epi16
	// _mm_cmpgt_epi32
	// _mm_cmpgt_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] > src2[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] > src2[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] > src2[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] > src2[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp>");
	BINARYOP_TEST_D("_mm_cmpgt_pd", _mm_cmpgt_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmpgt_sd", _mm_cmpgt_sd, _mm_load_pd(src), _mm_load_pd(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] <= src2[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] <= src2[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] <= src2[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] <= src2[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp<=");
	BINARYOP_TEST_D("_mm_cmple_pd", _mm_cmple_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmple_sd", _mm_cmple_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_cmplt_epi16
	// _mm_cmplt_epi32
	// _mm_cmplt_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] < src2[0]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] < src2[1]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] < src2[2]) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] < src2[3]) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp<");
	BINARYOP_TEST_D("_mm_cmplt_pd", _mm_cmplt_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmplt_sd", _mm_cmplt_sd, _mm_load_pd(src), _mm_load_pd(src2));

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

	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (!Isnan(dst[0]) && !Isnan(src2[0])) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (!Isnan(dst[1]) && !Isnan(src2[1])) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (!Isnan(dst[2]) && !Isnan(src2[2])) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (!Isnan(dst[3]) && !Isnan(src2[3])) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmpord");
	BINARYOP_TEST_D("_mm_cmpord_pd", _mm_cmpord_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmpord_sd", _mm_cmpord_sd, _mm_load_pd(src), _mm_load_pd(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (Isnan(dst[0]) || Isnan(src2[0])) ? ucastd(0xFFFFFFFFU) : 0.f; dst[1] = (Isnan(dst[1]) || Isnan(src2[1])) ? ucastd(0xFFFFFFFFU) : 0.f; dst[2] = (Isnan(dst[2]) || Isnan(src2[2])) ? ucastd(0xFFFFFFFFU) : 0.f; dst[3] = (Isnan(dst[3]) || Isnan(src2[3])) ? ucastd(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmpunord");
	BINARYOP_TEST_D("_mm_cmpunord_pd", _mm_cmpunord_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_cmpunord_sd", _mm_cmpunord_sd, _mm_load_pd(src), _mm_load_pd(src2));

	SETCHART("max");
	// _mm_max_epi16
	// _mm_max_epu8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = Max(dst[0], src2[0]); dst[1] = Max(dst[1], src2[1]); dst[2] = Max(dst[2], src2[2]); dst[3] = Max(dst[3], src2[3]); } ENDSCALAR(checksum_dst(dst), "scalar max");
	BINARYOP_TEST_D("_mm_max_pd", _mm_max_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_max_sd", _mm_max_sd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_min_epi16
	// _mm_min_epu8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = Min(dst[0], src2[0]); dst[1] = Min(dst[1], src2[1]); dst[2] = Min(dst[2], src2[2]); dst[3] = Min(dst[3], src2[3]); } ENDSCALAR(checksum_dst(dst), "scalar min");
	BINARYOP_TEST_D("_mm_min_pd", _mm_min_pd, _mm_load_pd(src), _mm_load_pd(src2));
	BINARYOP_TEST_D("_mm_min_sd", _mm_min_sd, _mm_load_pd(src), _mm_load_pd(src2));

	SETCHART("shuffle");
	// _mm_extract_epi16
	// _mm_insert_epi16
	// _mm_shuffle_epi32
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[3] = dst[1]; dst[2] = dst[0]; dst[1] = src2[3]; dst[0] = src2[2]; } ENDSCALAR(checksum_dst(dst), "scalar shuffle");
//	BINARYOP_TEST_D("_mm_shuffle_pd", _mm_shuffle_pd, _mm_load_pd(src), _mm_load_pd(src2));
	START();
		__m128 o0 = _mm_load_pd(src);
		__m128 o1 = _mm_load_pd(src2);
		for(int i = 0; i < N; i += 4)
			o0 = _mm_shuffle_pd(o0, o1, _MM_SHUFFLE(1, 0, 3, 2));
		_mm_store_pd(dst, o0);
	END(checksum_dst(dst), "_mm_shuffle_pd");

	// _mm_shufflehi_epi16
	// _mm_shufflelo_epi16
	// _mm_unpackhi_epi16
	// _mm_unpackhi_epi32
	// _mm_unpackhi_epi64
	// _mm_unpackhi_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = dst[2]; dst[1] = src2[2]; dst[2] = dst[3]; dst[3] = src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar unpackhi_pd");
	BINARYOP_TEST_D("_mm_unpackhi_pd", _mm_unpackhi_pd, _mm_load_pd(src), _mm_load_pd(src2));
	// _mm_unpacklo_epi16
	// _mm_unpacklo_epi32
	// _mm_unpacklo_epi64
	// _mm_unpacklo_epi8
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[2] = dst[1]; dst[1] = dst[0]; dst[0] = src2[0]; dst[3] = src2[1]; } ENDSCALAR(checksum_dst(dst), "scalar unpacklo_pd");
	BINARYOP_TEST_D("_mm_unpacklo_pd", _mm_unpacklo_pd, _mm_load_pd(src), _mm_load_pd(src2));
	printf("]}\n");
/*
	printf("Finished!\n");
	printf("Total time spent in scalar intrinsics: %f msecs.\n", (double)scalarTotalTicks * 1000.0 / ticks_per_sec());
	printf("Total time spent in SSE1 intrinsics: %f msecs.\n", (double)simdTotalTicks * 1000.0 / ticks_per_sec());
	if (scalarTotalTicks > simdTotalTicks)
		printf("SSE1 was %.3fx faster than scalar!\n", (double)scalarTotalTicks / simdTotalTicks);
	else
		printf("SSE1 was %.3fx slower than scalar!\n", (double)simdTotalTicks / scalarTotalTicks);
*/
#ifdef __EMSCRIPTEN__
	fprintf(stderr,"User Agent: %s\n", emscripten_run_script_string("navigator.userAgent"));
	printf("/*Test finished! Now please close Firefox to continue with benchmark_sse2.py.*/\n");
#endif
	exit(0);
}
