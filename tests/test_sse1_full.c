// This file uses SSE1 by calling different functions with different interesting inputs and prints the results.
// Use a diff tool to compare the results between platforms.

#include <xmmintrin.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <float.h>
#include <assert.h>

// Recasts floating point representation of f to an integer.
uint32_t fcastu(float f) { return *(uint32_t*)&f; }
float ucastf(uint32_t t) { return *(float*)&t; }

// Data used in test. Store them global and access via a getter to confuse optimizer to not "solve" the whole test suite at compile-time,
// so that the operation will actually be performed at runtime, and not at compile-time. (Testing the capacity of the compiler to perform
// SIMD ops at compile-time would be interesting as well, but that's for another test)
float interesting_floats_[] = { -INFINITY, -FLT_MAX, -2.5f, -1.5f, -1.4f, -1.0f, -0.5f, -0.2f, -FLT_MIN, -0.f, 0.f, 
	                            1.401298464e-45f, FLT_MIN, 0.3f, 0.5f, 0.8f, 1.0f, 1.5f, 2.5f, 3.5f, 3.6f, FLT_MAX, INFINITY, NAN,
	                            ucastf(0x01020304), ucastf(0x80000000), ucastf(0x7FFFFFFF), ucastf(0xFFFFFFFF) };

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

bool IsNan(float f) { return (fcastu(f) << 1) > 0xFF000000u; }

char *SerializeFloat(float f, char *dstStr)
{
	if (!IsNan(f))
	{
		int numChars = sprintf(dstStr, "%.9g", f);
		return dstStr + numChars;
	}
	else
	{
		uint32_t u = fcastu(f);
		int numChars = sprintf(dstStr, "NaN(%8X)", (unsigned int)u);
		return dstStr + numChars;
	}
}

void tostr(__m128 *m, char *outstr)
{
	union { __m128 m; float val[4]; } u;
	u.m = *m;
	char s[4][32];
	SerializeFloat(u.val[0], s[0]);
	SerializeFloat(u.val[1], s[1]);
	SerializeFloat(u.val[2], s[2]);
	SerializeFloat(u.val[3], s[3]);
	sprintf(outstr, "[%s,%s,%s,%s]", s[3], s[2], s[1], s[0]);
}

// Accessors to the test data in a way that the compiler can't optimize at compile-time.
__attribute__((noinline)) float *get_arr()
{
	return always_true() ? interesting_floats_ : 0;
}

__m128 ExtractInRandomOrder(float *arr, int i, int n, int prime)
{
	return _mm_set_ps(arr[(i*prime)%n], arr[((i+1)*prime)%n], arr[((i+2)*prime)%n], arr[((i+3)*prime)%n]);
}

#define E1(arr, i, n) ExtractInRandomOrder(arr, i, n, 1)
#define E2(arr, i, n) ExtractInRandomOrder(arr, i, n, 1787)

#define M128_M128(func) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingFloats / 4; ++j) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				__m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
				__m128 ret = func(m1, m2); \
				char str[256], str2[256], str3[256]; \
				tostr(&m1, str); tostr(&m2, str2); tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}

#define M128_M128_int(func) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingFloats / 4; ++j) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				__m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
				int ret = func(m1, m2); \
				char str[256], str2[256]; \
				tostr(&m1, str); tostr(&m2, str2); \
				printf("%s(%s, %s) = %d\n", #func, str, str2, ret); \
			}

#define M128(func) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
		{ \
			__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
			__m128 ret = func(m1); \
			char str[256], str2[256]; \
			tostr(&m1, str); tostr(&ret, str2); \
			printf("%s(%s) = %s\n", #func, str, str2); \
		}

#define M128_M128_shuffle() \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingFloats / 4; ++j) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				__m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
				__m128 ret = _mm_shuffle_ps(m1, m2, _MM_SHUFFLE(1, 3, 0, 2)); \
				char str[256], str2[256], str3[256]; \
				tostr(&m1, str); tostr(&m2, str2); tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", "_mm_shuffle_ps", str, str2, str3); \
			}

int main()
{
	float *interesting_floats = get_arr();
	int numInterestingFloats = sizeof(interesting_floats_)/sizeof(interesting_floats_[0]);
	assert(numInterestingFloats % 4 == 0);

	// SSE1 Arithmetic instructions:
	M128_M128(_mm_add_ps);
	M128_M128(_mm_add_ss);
	M128_M128(_mm_div_ps);
	M128_M128(_mm_div_ss);
	M128_M128(_mm_mul_ps);
	M128_M128(_mm_mul_ss);
	M128_M128(_mm_sub_ps);
	M128_M128(_mm_sub_ss);

	// SSE1 Elementary Math functions:
	M128(_mm_rcp_ps);
	M128(_mm_rcp_ss);
	M128(_mm_rsqrt_ps);
	M128(_mm_rsqrt_ss);
	M128(_mm_sqrt_ps);
	M128(_mm_sqrt_ss);

	// SSE1 Logical instructions:
	M128_M128(_mm_and_ps);
	M128_M128(_mm_andnot_ps);
	M128_M128(_mm_or_ps);
	M128_M128(_mm_xor_ps);

	// SSE1 Compare instructions:
	M128_M128(_mm_cmpeq_ps);
	M128_M128(_mm_cmpeq_ss);
	M128_M128(_mm_cmpge_ps);
	M128_M128(_mm_cmpge_ss);
	M128_M128(_mm_cmpgt_ps);
	M128_M128(_mm_cmpgt_ss);
	M128_M128(_mm_cmple_ps);
	M128_M128(_mm_cmple_ss);
	M128_M128(_mm_cmplt_ps);
	M128_M128(_mm_cmplt_ss);
	M128_M128(_mm_cmpneq_ps);
	M128_M128(_mm_cmpneq_ss);
	M128_M128(_mm_cmpnge_ps);
	M128_M128(_mm_cmpnge_ss);
	M128_M128(_mm_cmpngt_ps);
	M128_M128(_mm_cmpngt_ss);
	M128_M128(_mm_cmpnle_ps);
	M128_M128(_mm_cmpnle_ss);
	M128_M128(_mm_cmpnlt_ps);
	M128_M128(_mm_cmpnlt_ss);
	M128_M128(_mm_cmpord_ps);
	M128_M128(_mm_cmpord_ss);
	M128_M128(_mm_cmpunord_ps);
	M128_M128(_mm_cmpunord_ss);

	M128_M128_int(_mm_comieq_ss);
	M128_M128_int(_mm_comige_ss);
	M128_M128_int(_mm_comigt_ss);
	M128_M128_int(_mm_comile_ss);
	M128_M128_int(_mm_comilt_ss);
	M128_M128_int(_mm_comineq_ss);
	M128_M128_int(_mm_ucomieq_ss);
	M128_M128_int(_mm_ucomige_ss);
	M128_M128_int(_mm_ucomigt_ss);
	M128_M128_int(_mm_ucomile_ss);
	M128_M128_int(_mm_ucomilt_ss);
	M128_M128_int(_mm_ucomineq_ss);

/*
	// SSE1 Convert instructions:
	_mm_cvt_si2ss;
	_mm_cvt_ss2si;
	_mm_cvtsi32_ss;
	_mm_cvtss_f32;
	_mm_cvtss_si32;
	_mm_cvtss_si64;
	_mm_cvtt_ss2si;
	_mm_cvttss_si32;
	_mm_cvttss_si64;
*/

/*
	// SSE1 Load functions:
	_mm_load_ps
	_mm_load_ps1
	_mm_load_ss
	_mm_load1_ps
	_mm_loadh_pi
	_mm_loadl_pi
	_mm_loadr_ps
	_mm_loadu_ps
*/

	// SSE1 Miscellaneous functions:
//	_mm_movemask_ps

	// SSE1 Move functions:
	M128_M128(_mm_move_ss);
	M128_M128(_mm_movehl_ps);
	M128_M128(_mm_movelh_ps);

/*
	// SSE1 Set functions:
	_mm_set_ps
	_mm_set_ps1
	_mm_set_ss
	_mm_set1_ps
	_mm_setr_ps
	_mm_setzero_ps
*/

	// SSE1 Special Math instructions:
	M128_M128(_mm_max_ps);
	M128_M128(_mm_max_ss);
	M128_M128(_mm_min_ps);
	M128_M128(_mm_min_ss);

/*
	// SSE1 Store instructions:
	_mm_store_ps
	_mm_store_ps1
	_mm_store_ss
	_mm_store1_ps
	_mm_storeh_pi
	_mm_storel_pi
	_mm_storer_ps
	_mm_storeu_ps
	_mm_stream_pi
	_mm_stream_ps
*/

	// SSE1 Swizzle instructions:
	M128_M128_shuffle();
	// _MM_TRANSPOSE4_PS
	M128_M128(_mm_unpackhi_ps);
	M128_M128(_mm_unpacklo_ps);
}
