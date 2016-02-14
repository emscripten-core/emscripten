#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <float.h>
#include <assert.h>
#ifdef _WIN32
#include <string>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define align1_int emscripten_align1_int
#define align1_int64 emscripten_align1_int64
#define align1_float emscripten_align1_float
#define align1_double emscripten_align1_double
#else
#define align1_int64 int64_t
#define align1_int int
#define align1_float float
#define align1_double double
#endif

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
#if 0 // TODO: SIMD.js canonicalizes floats, which breaks using bit patterns in m128.
                                ucastf(0x01020304), ucastf(0x80000000), ucastf(0x7FFFFFFF), ucastf(0xFFFFFFFF)
#endif
                            };

double interesting_doubles_[] = { -INFINITY, -FLT_MAX, -2.5, -1.5, -1.4, -1.0, -0.5, -0.2, -FLT_MIN, -0.0, 0.0, 
                                1.401298464e-45, FLT_MIN, 0.3, 0.5, 0.8, 1.0, 1.5, 2.5, 3.5, 3.6, FLT_MAX, INFINITY, NAN,
#if 0 // TODO: SIMD.js canonicalizes floats, which breaks using bit patterns in m128.
                                ucastd(0x0102030405060708ULL), ucastd(0x8000000000000000ULL), ucastd(0x7FFFFFFFFFFFFFFFULL), ucastd(0xFFFFFFFFFFFFFFFFULL)
#endif
                                };

uint32_t interesting_ints_[] = { 0, 1, 2, 3, 0x01020304, 0x10203040, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0x12345678, 0x9ABCDEF1, 0x80000000,
                                 0x80808080, 0x7F7F7F7F, 0x01010101, 0x11111111, 0x20202020, 0x0F0F0F0F, 0xF0F0F0F0,
                                 fcastu(-INFINITY), fcastu(-FLT_MAX), fcastu(-2.5f), fcastu(-1.5f), fcastu(-1.4f), fcastu(-1.0f), fcastu(-0.5f),
                                 fcastu(-0.2f), fcastu(-FLT_MIN), 0xF9301AB9, 0x0039AB12, 0x19302BCD,
                                 fcastu(1.401298464e-45f), fcastu(FLT_MIN), fcastu(0.3f), fcastu(0.5f), fcastu(0.8f), fcastu(1.0f), fcastu(1.5f),
                                 fcastu(2.5f), fcastu(3.5f), fcastu(3.6f), fcastu(FLT_MAX), fcastu(INFINITY), fcastu(NAN) };

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

bool IsNan(float f) { return (fcastu(f) << 1) > 0xFF000000u; }

#ifdef _WIN32
std::string replace(std::string str, std::string a, std::string b)
{
	size_t index = 0;
	while(true)
	{
		index = str.find(a, index);
		if (index == std::string::npos) break;
		str.replace(index, a.length(), b);
		index += b.length();
	}
	return str;
}

// sprintf on Windows prints floats a bit differently, but since we
// are using Clang to compile and not MSVC, we don't seem to have access
// to the Win32-specific MSVC runtime functions to adjust the output.
// Therefore just be brute and hacky about unifying the result.
std::string WinHackCanonicalizeStringComparisons(std::string s)
{
	s = replace(s, "e+0", "e+");
	s = replace(s, "e-0", "e-");
	s = replace(s, "1.#INF", "inf");
	return s;
}
#endif

char *SerializeFloat(float f, char *dstStr)
{
	if (!IsNan(f))
	{
		int numChars = sprintf(dstStr, "%.9g", f);
#ifdef _WIN32
		std::string s = WinHackCanonicalizeStringComparisons(dstStr);
		numChars = sprintf(dstStr, "%s", s.c_str());
#endif		
		return dstStr + numChars;
	}
	else
	{
		uint32_t u = fcastu(f);
#if 0 // TODO: SIMD.js canonicalizes floats, which breaks using bit patterns in m128.
		int numChars = sprintf(dstStr, "NaN(0x%8X)", (unsigned int)u);
#else
		int numChars = sprintf(dstStr, "NaN");
#endif
		return dstStr + numChars;
	}
}

char *SerializeDouble(double f, char *dstStr)
{
	if (!IsNan(f))
	{
		int numChars = sprintf(dstStr, "%.17g", f);
#ifdef _WIN32
		std::string s = WinHackCanonicalizeStringComparisons(dstStr);
		numChars = sprintf(dstStr, "%s", s.c_str());
#endif		
		return dstStr + numChars;
	}
	else
	{
		uint64_t u = dcastu(f);
#if 0 // TODO: SIMD.js canonicalizes floats, which breaks using bit patterns in m128.
		int numChars = sprintf(dstStr, "NaN(0x%08X%08X)", (unsigned int)(u>>32), (unsigned int)u);
#else
		int numChars = sprintf(dstStr, "NaN");
#endif
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

void tostr(align1_int *m, char *outstr)
{
	sprintf(outstr, "0x%08X", *m);
}

void tostr(align1_int64 *m, char *outstr)
{
	sprintf(outstr, "0x%08X%08X", (int)(*m >> 32), (int)*m);
}

void tostr(align1_float *m, char *outstr)
{
	SerializeFloat(*m, outstr);
}

void tostr(align1_double *m, char *outstr)
{
	SerializeDouble(*m, outstr);
}

void tostr(align1_double *m, int numElems, char *outstr)
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

void tostr(align1_float *m, int numElems, char *outstr)
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

void tostr(align1_int *s, int numElems, char *outstr)
{
	switch(numElems)
	{
		case 1: sprintf(outstr, "{0x%08X}", s[0]); break;
		case 2: sprintf(outstr, "{0x%08X,0x%08X}", s[0], s[1]); break;
		case 3: sprintf(outstr, "{0x%08X,0x%08X,0x%08X}", s[0], s[1], s[2]); break;
		case 4: sprintf(outstr, "{0x%08X,0x%08X,0x%08X,0x%08X}", s[0], s[1], s[2], s[3]); break;
	}
}

void tostr(align1_int64 *m, int numElems, char *outstr)
{
	switch(numElems)
	{
		case 1: sprintf(outstr, "{0x%08X%08X}", (int)(*m >> 32), (int)*m); break;
		case 2: sprintf(outstr, "{0x%08X%08X,0x%08X%08X}", (int)(*m >> 32), (int)*m, (int)(m[1] >> 32), (int)m[1]);
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

#define Ret_M128_Tint_body(Ret_type, func, Tint) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
		{ \
			__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
			Ret_type ret = func(m1, Tint); \
			char str[256]; tostr(&m1, str); \
			char str2[256]; tostr(&ret, str2); \
			printf("%s(%s, %d) = %s\n", #func, str, Tint, str2); \
		}

#define Ret_M128i_Tint_body(Ret_type, func, Tint) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
		{ \
			__m128i m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
			Ret_type ret = func(m1, Tint); \
			char str[256]; tostr(&m1, str); \
			char str2[256]; tostr(&ret, str2); \
			printf("%s(%s, %d) = %s\n", #func, str, Tint, str2); \
		}

#define Ret_M128i_int_Tint_body(Ret_type, func, Tint) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int j = 0; j < numInterestingInts; ++j) \
			for(int k = 0; k < 4; ++k) \
			{ \
				__m128i m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
				Ret_type ret = func(m1, interesting_ints[j], Tint); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&ret, str2); \
				printf("%s(%s, 0x%08X, %d) = %s\n", #func, str, interesting_ints[j], Tint, str2); \
			}

#define Ret_M128d_M128d_Tint_body(Ret_type, func, Tint) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int k = 0; k < 2; ++k) \
			for(int j = 0; j < numInterestingDoubles / 2; ++j) \
			{ \
				__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
				__m128d m2 = E2(interesting_doubles, j*2, numInterestingDoubles); \
				Ret_type ret = func(m1, m2, Tint); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
			}

#define Ret_M128i_M128i_Tint_body(Ret_type, func, Tint) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingInts / 4; ++j) \
			{ \
				__m128i m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
				__m128i m2 = E2(interesting_ints, j*4, numInterestingInts); \
				Ret_type ret = func(m1, m2, Tint); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
			}

#define Ret_M128_M128_Tint_body(Ret_type, func, Tint) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingFloats / 4; ++j) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				__m128 m2 = E2(interesting_floats, j*4, numInterestingFloats); \
				Ret_type ret = func(m1, m2, Tint); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s, %d) = %s\n", #func, str, str2, Tint, str3); \
			}

#define const_int8_unroll(Ret_type, F, func) \
	F(Ret_type, func, -1); \
	F(Ret_type, func, 0); \
	F(Ret_type, func, 1); \
	F(Ret_type, func, 2); \
	F(Ret_type, func, 3); \
	F(Ret_type, func, 5); \
	F(Ret_type, func, 7); \
	F(Ret_type, func, 11); \
	F(Ret_type, func, 13); \
	F(Ret_type, func, 15); \
	F(Ret_type, func, 16); \
	F(Ret_type, func, 17); \
	F(Ret_type, func, 23); \
	F(Ret_type, func, 29); \
	F(Ret_type, func, 31); \
	F(Ret_type, func, 37); \
	F(Ret_type, func, 43); \
	F(Ret_type, func, 47); \
	F(Ret_type, func, 59); \
	F(Ret_type, func, 127); \
	F(Ret_type, func, 128); \
	F(Ret_type, func, 191); \
	F(Ret_type, func, 254); \
	F(Ret_type, func, 255); \
	F(Ret_type, func, 309);

#define Ret_M128_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128_Tint_body, func)
#define Ret_M128i_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128i_Tint_body, func)
#define Ret_M128i_int_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128i_int_Tint_body, func)
#define Ret_M128i_M128i_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128i_M128i_Tint_body, func)
#define Ret_M128d_M128d_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128d_M128d_Tint_body, func)
#define Ret_M128_M128_Tint(Ret_type, func) const_int8_unroll(Ret_type, Ret_M128_M128_Tint_body, func)

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

#define Ret_M128d_int(Ret_type, func) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int k = 0; k < 2; ++k) \
			for(int j = 0; j < numInterestingInts; ++j) \
			{ \
				__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
				int m2 = interesting_ints[j]; \
				Ret_type ret = func(m1, m2); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}

#define Ret_M128d_int64(Ret_type, func) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int k = 0; k < 2; ++k) \
			for(int j = 0; j < numInterestingInts; ++j) \
				for(int l = 0; l < numInterestingInts; ++l) \
				{ \
					__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
					int64_t m2 = (int64_t)(((uint64_t)interesting_ints[j]) << 32 | (uint64_t)interesting_ints[l]); \
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

float tempOutFloatStore[16];
float *getTempOutFloatStore(int alignmentBytes)
{
	uintptr_t addr = (uintptr_t)tempOutFloatStore;
	addr = (addr + alignmentBytes - 1) & ~(alignmentBytes-1);
	return (float*)addr;
}

int *getTempOutIntStore(int alignmentBytes) { return (int*)getTempOutFloatStore(alignmentBytes); }
double *getTempOutDoubleStore(int alignmentBytes) { return (double*)getTempOutFloatStore(alignmentBytes); }

#define void_OutFloatPtr_M128(func, Ptr_type, numBytesWritten, alignmentBytes) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
			for(int k = 0; k < 4; ++k) \
			{ \
				uintptr_t base = (uintptr_t)getTempOutFloatStore(16); \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				align1_float *out = (align1_float*)(base + offset); \
				func((Ptr_type)out, m1); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(out, numBytesWritten/sizeof(float), str2); \
				printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
			}

#define void_OutDoublePtr_M128d(func, Ptr_type, numBytesWritten, alignmentBytes) \
	for(int i = 0; i < numInterestingDoubles / 2; ++i) \
		for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
			for(int k = 0; k < 2; ++k) \
			{ \
				uintptr_t base = (uintptr_t)getTempOutDoubleStore(16); \
				__m128d m1 = E1(interesting_doubles, i*2+k, numInterestingDoubles); \
				align1_double *out = (align1_double*)(base + offset); \
				func((Ptr_type)out, m1); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(out, numBytesWritten/sizeof(double), str2); \
				printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
			}

#define void_OutIntPtr_M128(func, Ptr_type, numBytesWritten, alignmentBytes) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
			for(int k = 0; k < 4; ++k) \
			{ \
				uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
				__m128 m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
				align1_int *out = (align1_int*)(base + offset); \
				func((Ptr_type)out, m1); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(out, numBytesWritten/sizeof(int), str2); \
				printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
			}

#define void_OutIntPtr_int(func, Ptr_type, numBytesWritten, alignmentBytes) \
	for(int i = 0; i < numInterestingInts; ++i) \
		for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
			for(int k = 0; k < 4; ++k) \
			{ \
				uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
				int m1 = interesting_ints[i]; \
				align1_int *out = (align1_int*)(base + offset); \
				func((Ptr_type)out, m1); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(out, numBytesWritten/sizeof(int), str2); \
				printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
			}

#define void_OutIntPtr_int64(func, Ptr_type, numBytesWritten, alignmentBytes) \
	for(int i = 0; i < numInterestingInts; ++i) \
		for(int j = 0; j < numInterestingInts; ++j) \
			for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
			{ \
				uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
				int64_t m1 = (int64_t)(((uint64_t)interesting_ints[i]) << 32 | (uint64_t)interesting_ints[j]); \
				align1_int64 *out = (align1_int64*)(base + offset); \
				func((Ptr_type)out, m1); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(out, numBytesWritten/sizeof(int64_t), str2); \
				printf("%s(p:align=%d, %s) = %s\n", #func, offset, str, str2); \
			}

#define void_M128i_M128i_OutIntPtr(func, Ptr_type, numBytesWritten, alignmentBytes) \
	for(int i = 0; i < numInterestingInts / 4; ++i) \
		for(int j = 0; j < numInterestingInts / 4; ++j) \
			for(int offset = 0; offset < numBytesWritten; offset += alignmentBytes) \
				for(int k = 0; k < 4; ++k) \
				{ \
					uintptr_t base = (uintptr_t)getTempOutIntStore(16); \
					__m128d m1 = E1(interesting_ints, i*4+k, numInterestingInts); \
					__m128i m2 = E2(interesting_ints, j*4, numInterestingInts); \
					align1_int *out = (int*)(base + offset); \
					func(m1, m2, (Ptr_type)out); \
					char str[256]; tostr(&m1, str); \
					char str2[256]; tostr(&m2, str2); \
					char str3[256]; tostr(out, numBytesWritten/sizeof(int), str3); \
					printf("%s(%s, %s, p:align=%d) = %s\n", #func, str, str2, offset, str3); \
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

#define Ret_M128_int(Ret_type, func) \
	for(int i = 0; i < numInterestingFloats / 4; ++i) \
		for(int k = 0; k < 4; ++k) \
			for(int j = 0; j < numInterestingInts; ++j) \
			{ \
				__m128 m1 = E1(interesting_floats, i*4+k, numInterestingFloats); \
				int m2 = interesting_ints[j]; \
				Ret_type ret = func(m1, m2); \
				char str[256]; tostr(&m1, str); \
				char str2[256]; tostr(&m2, str2); \
				char str3[256]; tostr(&ret, str3); \
				printf("%s(%s, %s) = %s\n", #func, str, str2, str3); \
			}
