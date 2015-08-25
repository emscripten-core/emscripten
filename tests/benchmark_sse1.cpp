#include <xmmintrin.h>
#include "benchmark_sse.h"

int main()
{
#ifndef __EMSCRIPTEN__
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif

	printf ("{ \"workload\": %u, \"results\": [\n", N);
	assert(N%4 == 0); // Don't care about the tail for now.
	float *src = get_src();//(float*)aligned_alloc(16, N*sizeof(float));
	for(int i = 0; i < N; ++i)
		src[i] = (float)rand() / RAND_MAX;
	float *src2 = get_src2();//(float*)aligned_alloc(16, N*sizeof(float));
	for(int i = 0; i < N; ++i)
		src2[i] = (float)rand() / RAND_MAX;
	float *dst = get_dst();//(float*)aligned_alloc(16, N*sizeof(float));

	float scalarTime;
	SETCHART("load");
	START();
		for(int i = 0; i < N; ++i)
			dst[i] = src[i];
	ENDSCALAR(checksum_dst(dst), "scalar");

	LS_TEST("_mm_load_ps", _mm_load_ps, 0, _mm_store_ps, float*, 0, 4);
	LS_TEST("_mm_load_ps1", _mm_load_ps1, 1, _mm_store_ps, float*, 0, 4);
	LS_TEST("_mm_load_ss", _mm_load_ss, 1, _mm_store_ps, float*, 0, 4);
	LS_TEST("_mm_load1_ps", _mm_load1_ps, 1, _mm_store_ps, float*, 0, 4);

	__m128 tempReg = _mm_set_ps(1.f, 2.f, 3.f, 4.f);
	LSH_TEST("_mm_loadh_pi", tempReg, _mm_loadh_pi, __m64*, 1, _mm_store_ps, float*, 0, 4);
	LSH_TEST("_mm_loadl_pi", tempReg, _mm_loadh_pi, __m64*, 1, _mm_store_ps, float*, 0, 4);

	LS_TEST("_mm_loadr_ps", _mm_loadr_ps, 0, _mm_store_ps, float*, 0, 4);
	LS_TEST("_mm_loadu_ps", _mm_loadu_ps, 1, _mm_store_ps, float*, 0, 4);

	SETCHART("set");
	SS_TEST("_mm_set_ps", _mm_set_ps(src[i+2], src[i+1], src[i+5], src[i+0]));
	SS_TEST("_mm_set_ps1", _mm_set_ps1(src[i]));
	SS_TEST("_mm_set_ss", _mm_set_ss(src[i]));
	SS_TEST("_mm_set1_ps", _mm_set1_ps(src[i]));
	SS_TEST("_mm_setr_ps", _mm_set_ps(src[i+2], src[i+1], src[i+5], src[i+0]));
	SS_TEST("_mm_setzero_ps", _mm_setzero_ps());

	SETCHART("move");
	SS_TEST("_mm_move_ss", _mm_move_ss(_mm_load_ps(src+i), _mm_load_ps(src2+i)));
	SS_TEST("_mm_movehl_ps", _mm_movehl_ps(_mm_load_ps(src+i), _mm_load_ps(src2+i)));
	SS_TEST("_mm_movelh_ps", _mm_movelh_ps(_mm_load_ps(src+i), _mm_load_ps(src2+i)));

	SETCHART("store");
	LS_TEST("_mm_store_ps", _mm_load_ps, 0, _mm_store_ps, float*, 0, 4);
	LS_TEST("_mm_store_ps1", _mm_load_ps, 0, _mm_store_ps1, float*, 0, 4);
	LS_TEST("_mm_store_ss", _mm_load_ps, 0, _mm_store_ss, float*, 1, 4);
	LS64_TEST("_mm_storeh_pi", _mm_load_ps, 0, _mm_storeh_pi, 1, 4);
	LS64_TEST("_mm_storel_pi", _mm_load_ps, 0, _mm_storel_pi, 1, 4);
	LS_TEST("_mm_storer_ps", _mm_load_ps, 0, _mm_storer_ps, float*, 0, 4);
	LS_TEST("_mm_storeu_ps", _mm_load_ps, 0, _mm_storeu_ps, float*, 1, 4);
	LS_TEST("_mm_stream_ps", _mm_load_ps, 0, _mm_stream_ps, float*, 0, 4);

	SETCHART("arithmetic");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] += src2[0]; dst[1] += src2[1]; dst[2] += src2[2]; dst[3] += src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar add");
	BINARYOP_TEST("_mm_add_ps", _mm_add_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_add_ss", _mm_add_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] /= src2[0]; dst[1] /= src2[1]; dst[2] /= src2[2]; dst[3] /= src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar div");
	BINARYOP_TEST("_mm_div_ps", _mm_div_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_div_ss", _mm_div_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] *= src2[0]; dst[1] *= src2[1]; dst[2] *= src2[2]; dst[3] *= src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar mul");
	BINARYOP_TEST("_mm_mul_ps", _mm_mul_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_mul_ss", _mm_mul_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] -= src2[0]; dst[1] -= src2[1]; dst[2] -= src2[2]; dst[3] -= src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar sub");
	BINARYOP_TEST("_mm_sub_ps", _mm_sub_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_sub_ss", _mm_sub_ss, _mm_load_ps(src), _mm_load_ps(src2));

	SETCHART("roots");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = 1.f / dst[0]; dst[1] = 1.f / dst[1]; dst[2] = 1.f / dst[2]; dst[3] = 1.f / dst[3]; } ENDSCALAR(checksum_dst(dst), "scalar rcp");
	UNARYOP_TEST("_mm_rcp_ps", _mm_rcp_ps, _mm_load_ps(src));
	UNARYOP_TEST("_mm_rcp_ss", _mm_rcp_ss, _mm_load_ps(src));

	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = 1.f / sqrtf(dst[0]); dst[1] = 1.f / sqrtf(dst[1]); dst[2] = 1.f / sqrtf(dst[2]); dst[3] = 1.f / sqrtf(dst[3]); } ENDSCALAR(checksum_dst(dst), "scalar rsqrt");
	UNARYOP_TEST("_mm_rsqrt_ps", _mm_rsqrt_ps, _mm_load_ps(src));
	UNARYOP_TEST("_mm_rsqrt_ss", _mm_rsqrt_ss, _mm_load_ps(src));

	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = sqrtf(dst[0]); dst[1] = sqrtf(dst[1]); dst[2] = sqrtf(dst[2]); dst[3] = sqrtf(dst[3]); } ENDSCALAR(checksum_dst(dst), "scalar sqrt");
	UNARYOP_TEST("_mm_sqrt_ps", _mm_sqrt_ps, _mm_load_ps(src));
	UNARYOP_TEST("_mm_sqrt_ss", _mm_sqrt_ss, _mm_load_ps(src));

	SETCHART("logical");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastf(fcastu(dst[0]) & fcastu(src2[0])); dst[1] = ucastf(fcastu(dst[1]) & fcastu(src2[1])); dst[2] = ucastf(fcastu(dst[2]) & fcastu(src2[2])); dst[3] = ucastf(fcastu(dst[3]) & fcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar and");
	BINARYOP_TEST("_mm_and_ps", _mm_and_ps, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastf((~fcastu(dst[0])) & fcastu(src2[0])); dst[1] = ucastf((~fcastu(dst[1])) & fcastu(src2[1])); dst[2] = ucastf((~fcastu(dst[2])) & fcastu(src2[2])); dst[3] = ucastf((~fcastu(dst[3])) & fcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar andnot");
	BINARYOP_TEST("_mm_andnot_ps", _mm_andnot_ps, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastf(fcastu(dst[0]) | fcastu(src2[0])); dst[1] = ucastf(fcastu(dst[1]) | fcastu(src2[1])); dst[2] = ucastf(fcastu(dst[2]) | fcastu(src2[2])); dst[3] = ucastf(fcastu(dst[3]) | fcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar or");
	BINARYOP_TEST("_mm_or_ps", _mm_or_ps, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = ucastf(fcastu(dst[0]) ^ fcastu(src2[0])); dst[1] = ucastf(fcastu(dst[1]) ^ fcastu(src2[1])); dst[2] = ucastf(fcastu(dst[2]) ^ fcastu(src2[2])); dst[3] = ucastf(fcastu(dst[3]) ^ fcastu(src2[3])); } ENDSCALAR(checksum_dst(dst), "scalar xor");
	BINARYOP_TEST("_mm_xor_ps", _mm_xor_ps, _mm_load_ps(src), _mm_load_ps(src2));

	SETCHART("cmp");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] == src2[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] == src2[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] == src2[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] == src2[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp==");
	BINARYOP_TEST("_mm_cmpeq_ps", _mm_cmpeq_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmpeq_ss", _mm_cmpeq_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] >= src2[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] >= src2[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] >= src2[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] >= src2[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp>=");
	BINARYOP_TEST("_mm_cmpge_ps", _mm_cmpge_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmpge_ss", _mm_cmpge_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] > src2[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] > src2[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] > src2[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] > src2[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp>");
	BINARYOP_TEST("_mm_cmpgt_ps", _mm_cmpgt_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmpgt_ss", _mm_cmpgt_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] <= src2[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] <= src2[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] <= src2[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] <= src2[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp<=");
	BINARYOP_TEST("_mm_cmple_ps", _mm_cmple_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmple_ss", _mm_cmple_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (dst[0] < src2[0]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (dst[1] < src2[1]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (dst[2] < src2[2]) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (dst[3] < src2[3]) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmp<");
	BINARYOP_TEST("_mm_cmplt_ps", _mm_cmplt_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmplt_ss", _mm_cmplt_ss, _mm_load_ps(src), _mm_load_ps(src2));

	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (!Isnan(dst[0]) && !Isnan(src2[0])) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (!Isnan(dst[1]) && !Isnan(src2[1])) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (!Isnan(dst[2]) && !Isnan(src2[2])) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (!Isnan(dst[3]) && !Isnan(src2[3])) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmpord");
	BINARYOP_TEST("_mm_cmpord_ps", _mm_cmpord_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmpord_ss", _mm_cmpord_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = (Isnan(dst[0]) || Isnan(src2[0])) ? ucastf(0xFFFFFFFFU) : 0.f; dst[1] = (Isnan(dst[1]) || Isnan(src2[1])) ? ucastf(0xFFFFFFFFU) : 0.f; dst[2] = (Isnan(dst[2]) || Isnan(src2[2])) ? ucastf(0xFFFFFFFFU) : 0.f; dst[3] = (Isnan(dst[3]) || Isnan(src2[3])) ? ucastf(0xFFFFFFFFU) : 0.f; } ENDSCALAR(checksum_dst(dst), "scalar cmpunord");
	BINARYOP_TEST("_mm_cmpunord_ps", _mm_cmpunord_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_cmpunord_ss", _mm_cmpunord_ss, _mm_load_ps(src), _mm_load_ps(src2));

	SETCHART("max");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = Max(dst[0], src2[0]); dst[1] = Max(dst[1], src2[1]); dst[2] = Max(dst[2], src2[2]); dst[3] = Max(dst[3], src2[3]); } ENDSCALAR(checksum_dst(dst), "scalar max");
	BINARYOP_TEST("_mm_max_ps", _mm_max_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_max_ss", _mm_max_ss, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = Min(dst[0], src2[0]); dst[1] = Min(dst[1], src2[1]); dst[2] = Min(dst[2], src2[2]); dst[3] = Min(dst[3], src2[3]); } ENDSCALAR(checksum_dst(dst), "scalar min");
	BINARYOP_TEST("_mm_min_ps", _mm_min_ps, _mm_load_ps(src), _mm_load_ps(src2));
	BINARYOP_TEST("_mm_min_ss", _mm_min_ss, _mm_load_ps(src), _mm_load_ps(src2));

	SETCHART("shuffle");
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[3] = dst[1]; dst[2] = dst[0]; dst[1] = src2[3]; dst[0] = src2[2]; } ENDSCALAR(checksum_dst(dst), "scalar shuffle");
//	BINARYOP_TEST("_mm_shuffle_ps", _mm_shuffle_ps, _mm_load_ps(src), _mm_load_ps(src2));
	START();
		__m128 o0 = _mm_load_ps(src);
		__m128 o1 = _mm_load_ps(src2);
		for(int i = 0; i < N; i += 4)
			o0 = _mm_shuffle_ps(o0, o1, _MM_SHUFFLE(1, 0, 3, 2));
		_mm_store_ps(dst, o0);
	END(checksum_dst(dst), "_mm_shuffle_ps");

	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[0] = dst[2]; dst[1] = src2[2]; dst[2] = dst[3]; dst[3] = src2[3]; } ENDSCALAR(checksum_dst(dst), "scalar unpackhi_ps");
	BINARYOP_TEST("_mm_unpackhi_ps", _mm_unpackhi_ps, _mm_load_ps(src), _mm_load_ps(src2));
	START(); dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; for(int i = 0; i < N; ++i) { dst[2] = dst[1]; dst[1] = dst[0]; dst[0] = src2[0]; dst[3] = src2[1]; } ENDSCALAR(checksum_dst(dst), "scalar unpacklo_ps");
	BINARYOP_TEST("_mm_unpacklo_ps", _mm_unpacklo_ps, _mm_load_ps(src), _mm_load_ps(src2));
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
	printf("/*Test finished! Now please close Firefox to continue with benchmark_sse1.py.*/\n");
#endif
	exit(0);
}
