#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <float.h>
#include <assert.h>

// Recasts floating point representation of f to an integer.
uint32_t fcastu(float f) { return *(uint32_t*)&f; }
uint64_t dcastu(double f) { return *(uint64_t*)&f; }
float ucastf(uint32_t t) { return *(float*)&t; }
double ucastd(uint64_t t) { return *(double*)&t; }

// Data used in test. Store them global and access via a getter to confuse optimizer to not "solve" the whole test suite at compile-time,
// so that the operation will actually be performed at runtime, and not at compile-time. (Testing the capacity of the compiler to perform
// SIMD ops at compile-time would be interesting as well, but that's for another test)
float interesting_floats_[] = { -INFINITY, -FLT_MAX, -2.5f, -1.5f, -1.4f, -1.0f, -0.5f, -0.2f, -FLT_MIN, -0.f, 0.f, 
                                1.401298464e-45f, FLT_MIN, 0.3f, 0.5f, 0.8f, 1.0f, 1.5f, 2.5f, 3.5f, 3.6f, FLT_MAX, INFINITY, NAN,
                                ucastf(0x01020304), ucastf(0x80000000), ucastf(0x7FFFFFFF), ucastf(0xFFFFFFFF) };

double interesting_doubles_[] = { -INFINITY, -FLT_MAX, -2.5, -1.5, -1.4, -1.0, -0.5, -0.2, -FLT_MIN, -0.0, 0.0, 
                                1.401298464e-45, FLT_MIN, 0.3, 0.5, 0.8, 1.0, 1.5, 2.5, 3.5, 3.6, FLT_MAX, INFINITY, NAN,
                                ucastd(0x0102030405060708ULL), ucastd(0x8000000000000000ULL), ucastd(0x7FFFFFFFFFFFFFFFULL), ucastd(0xFFFFFFFFFFFFFFFFULL) };

uint32_t interesting_ints_[] = { 0, 1, 2, 3, 0x01020304, 0x10203040, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0x12345678, 0x9ABCDEF1, 0x80000000,
                                 0x80808080, 0x7F7F7F7F, 0x01010101, 0x11111111, 0x20202020, 0x0F0F0F0F, 0xF0F0F0F0,
                                 fcastu(-INFINITY), fcastu(-FLT_MAX), fcastu(-2.5f), fcastu(-1.5f), fcastu(-1.4f), fcastu(-1.0f), fcastu(-0.5f),
                                 fcastu(-0.2f), fcastu(-FLT_MIN), 0xF9301AB9, 0x0039AB12, 0x19302BCD,
                                 fcastu(1.401298464e-45f), fcastu(FLT_MIN), fcastu(0.3f), fcastu(0.5f), fcastu(0.8f), fcastu(1.0f), fcastu(1.5f),
                                 fcastu(2.5f), fcastu(3.5f), fcastu(3.6f), fcastu(FLT_MAX), fcastu(INFINITY), fcastu(NAN) };

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
		int numChars = sprintf(dstStr, "NaN(0x%8X)", (unsigned int)u);
		return dstStr + numChars;
	}
}

char *SerializeDouble(double f, char *dstStr)
{
	if (!IsNan(f))
	{
		int numChars = sprintf(dstStr, "%.17g", f);
		return dstStr + numChars;
	}
	else
	{
		uint64_t u = dcastu(f);
		int numChars = sprintf(dstStr, "NaN(0x%08X%08X)", (unsigned int)(u>>32), (unsigned int)u);
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

#ifdef ENABLE_SSE2

void tostr(__m128i *m, char *outstr)
{
	union { __m128i m; uint32_t val[4]; } u;
	u.m = *m;
	sprintf(outstr, "[0x%08X,0x%08X,0x%08X,0x%08X]", u.val[3], u.val[2], u.val[1], u.val[0]);
}

void tostr(__m128d *m, char *outstr)
{
	union { __m128d m; double val[2]; } u;
	u.m = *m;
	char s[2][64];
	SerializeDouble(u.val[0], s[0]);
	SerializeDouble(u.val[1], s[1]);
	sprintf(outstr, "[%s,%s]", s[1], s[0]);
}

__m128i ExtractInRandomOrder(uint32_t *arr, int i, int n, int prime)
{
	return _mm_set_epi32(arr[(i*prime)%n], arr[((i+1)*prime)%n], arr[((i+2)*prime)%n], arr[((i+3)*prime)%n]);
}

__m128d ExtractInRandomOrder(double *arr, int i, int n, int prime)
{
	return _mm_set_pd(arr[(i*prime)%n], arr[((i+1)*prime)%n]);
}
#endif

void tostr(int *m, char *outstr)
{
	sprintf(outstr, "0x%08X", *m);
}

void tostr(int64_t *m, char *outstr)
{
	sprintf(outstr, "0x%08X%08X", (int)(*m >> 32), (int)*m);
}

void tostr(float *m, char *outstr)
{
	SerializeFloat(*m, outstr);
}

void tostr(double *m, char *outstr)
{
	SerializeDouble(*m, outstr);
}

void tostr(double *m, int numElems, char *outstr)
{
	char s[2][64];
	for(int i = 0; i < numElems; ++i)
		SerializeDouble(m[i], s[i]);
	switch(numElems)
	{
		case 1: sprintf(outstr, "{%s}", s[0]); break;
		case 2: sprintf(outstr, "{%s,%s}", s[0], s[1]); break;
	}
}

void tostr(float *m, int numElems, char *outstr)
{
	char s[4][64];
	for(int i = 0; i < numElems; ++i)
		SerializeFloat(m[i], s[i]);
	switch(numElems)
	{
		case 1: sprintf(outstr, "{%s}", s[0]); break;
		case 2: sprintf(outstr, "{%s,%s}", s[0], s[1]); break;
		case 3: sprintf(outstr, "{%s,%s,%s}", s[0], s[1], s[2]); break;
		case 4: sprintf(outstr, "{%s,%s,%s,%s}", s[0], s[1], s[2], s[3]); break;
	}
}

void tostr(int *s, int numElems, char *outstr)
{
	switch(numElems)
	{
		case 1: sprintf(outstr, "{0x%08X}", s[0]); break;
		case 2: sprintf(outstr, "{0x%08X,0x%08X}", s[0], s[1]); break;
		case 3: sprintf(outstr, "{0x%08X,0x%08X,0x%08X}", s[0], s[1], s[2]); break;
		case 4: sprintf(outstr, "{0x%08X,0x%08X,0x%08X,0x%08X}", s[0], s[1], s[2], s[3]); break;
	}
}

// Accessors to the test data in a way that the compiler can't optimize at compile-time.
__attribute__((noinline)) float *get_interesting_floats()
{
	return always_true() ? interesting_floats_ : 0;
}

__attribute__((noinline)) uint32_t *get_interesting_ints()
{
	return always_true() ? interesting_ints_ : 0;
}

__attribute__((noinline)) double *get_interesting_doubles()
{
	return always_true() ? interesting_doubles_ : 0;
}

__m128 ExtractInRandomOrder(float *arr, int i, int n, int prime)
{
	return _mm_set_ps(arr[(i*prime)%n], arr[((i+1)*prime)%n], arr[((i+2)*prime)%n], arr[((i+3)*prime)%n]);
}

#define E1(arr, i, n) ExtractInRandomOrder(arr, i, n, 1)
#define E2(arr, i, n) ExtractInRandomOrder(arr, i, n, 1787)

#define M128i_M128i_M128i(func) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingInts / 4; ++j) \
			{ \
				__m128i m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
				__m128i m2 = E2(interesting_ints, j*4, numInterestingInts); \
				__m128i ret = func(m1, m2); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}

#define Ret_M128d_M128d(Ret_type, func) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int k = 0; k < 2; ++k) \
			for(int j = 0; j < numInterestingDoubles / 2; ++j) \
			{ \
				__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
				__m128d m2 = E2(interesting_doubles, j*2, numInterestingDoubles); \
				Ret_type ret = func(m1, m2); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}

#define Ret_M128d(Ret_type, func) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int k = 0; k < 2; ++k) \
		{ \
			__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
			Ret_type ret = func(m1); \
			char str[256]; tostr(&m1, str); \
			char str2[256]; tostr(&ret, str2); \
			printf("%s(%s) = %s\n", #func, str, str2); \
		}

#define Ret_DoublePtr(Ret_type, func, numElemsAccessed, inc) \
	for(int i = 0; i+numElemsAccessed <= numInterestingDoubles; i += inc) \
	{ \
		double *ptr = interesting_doubles + i; \
		Ret_type ret = func(ptr); \
		char str[256]; tostr(ptr, numElemsAccessed, str); \
		char str2[256]; tostr(&ret, str2); \
		printf("%s(%s) = %s\n", #func, str, str2); \
	}

#define Ret_M128(Ret_type, func) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
		{ \
			__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
			Ret_type ret = func(m1); \
			char str[256]; tostr(&m1, str); \
			char str2[256]; tostr(&ret, str2); \
			printf("%s(%s) = %s\n", #func, str, str2); \
		}

#define Ret_FloatPtr(Ret_type, func, numElemsAccessed, inc) \
	for(int i = 0; i+numElemsAccessed <= numInterestingFloats; i += inc) \
	{ \
		float *ptr = interesting_floats + i; \
		Ret_type ret = func(ptr); \
		char str[256]; tostr(ptr, numElemsAccessed, str); \
		char str2[256]; tostr(&ret, str2); \
		printf("%s(%s) = %s\n", #func, str, str2); \
	}

#define Ret_IntPtr(Ret_type, func, Ptr_type, numElemsAccessed, inc) \
	for(int i = 0; i+numElemsAccessed <= numInterestingInts; i += inc) \
	{ \
		uint32_t *ptr = interesting_ints + i; \
		Ret_type ret = func((Ptr_type)ptr); \
		char str[256]; tostr((int*)ptr, numElemsAccessed, str); \
		char str2[256]; tostr(&ret, str2); \
		printf("%s(%s) = %s\n", #func, str, str2); \
	}

#define Ret_M128_FloatPtr(Ret_type, func, Ptr_type, numElemsAccessed, inc) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j+numElemsAccessed <= numInterestingFloats; j += inc) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				float *ptr = interesting_floats + j; \
				Ret_type ret = func(m1, (Ptr_type)ptr); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(ptr, numElemsAccessed, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}

#define Ret_M128d_DoublePtr(Ret_type, func, Ptr_type, numElemsAccessed, inc) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int k = 0; k < 2; ++k) \
			for(int j = 0; j+numElemsAccessed <= numInterestingDoubles; j += inc) \
			{ \
				__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
				double *ptr = interesting_doubles + j; \
				Ret_type ret = func(m1, (Ptr_type)ptr); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(ptr, numElemsAccessed, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}

#define Ret_M128i(Ret_type, func) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
		{ \
			__m128i m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
			Ret_type ret = func(m1); \
			char str[256]; tostr(&m1, str); \
			char str2[256]; tostr(&ret, str2); \
			printf("%s(%s) = %s\n", #func, str, str2); \
		}

#define Ret_int(Ret_type, func) \
	for(int i = 0; i < numInterestingInts; ++i) \
	{ \
		Ret_type ret = func(interesting_ints[i]); \
		char str[256]; tostr((int*)&interesting_ints[i], str); \
		char str2[256]; tostr(&ret, str2); \
		printf("%s(%s) = %s\n", #func, str, str2); \
	}

#define Ret_int64(Ret_type, func) \
	for(int i = 0; i < numInterestingInts; ++i) \
		for(int j = 0; j < numInterestingInts; ++j) \
		{ \
			int64_t m1 = (int64_t)(((uint64_t)interesting_ints[i]) << 32 | (uint64_t)interesting_ints[j]); \
			Ret_type ret = func(m1); \
			char str[256]; tostr(&m1, str); \
			char str2[256]; tostr(&ret, str2); \
			printf("%s(%s) = %s\n", #func, str, str2); \
		}

#define Ret_M128_M128(Ret_type, func) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingFloats / 4; ++j) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				__m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
				Ret_type ret = func(m1, m2); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
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
