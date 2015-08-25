#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
// This test never frees, so we can be carefree and just round up to be aligned.
#define aligned_alloc(align, size) (void*)(((uintptr_t)malloc((size) + ((align)-1)) + ((align)-1)) & (~((align)-1)))
#endif

#if defined(__unix__) && !defined(__EMSCRIPTEN__) // Native build without Emscripten.
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#define tick_t unsigned long long
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#define aligned_alloc(align, size) malloc((size))
#endif

#ifdef WIN32
#include <Windows.h>
#define tick_t unsigned long long
#define aligned_alloc(align, size) _aligned_malloc((size), (align))
#endif

// Scalar horizonal max across four lanes.
float hmax(__m128 m)
{
	float f[4];
	_mm_storeu_ps(f, m);
	return fmax(fmax(f[0], f[1]), fmax(f[2], f[3]));
}

#ifdef __EMSCRIPTEN__
#define tick emscripten_get_now
#define tick_t double
tick_t ticks_per_sec() { return 1000.0; }
#elif defined(__APPLE__)
#define tick_t unsigned long long
#define tick mach_absolute_time
tick_t ticks_per_sec()
{
	mach_timebase_info_data_t timeBaseInfo;
	mach_timebase_info(&timeBaseInfo);
	return 1000000000ULL * (uint64_t)timeBaseInfo.denom / (uint64_t)timeBaseInfo.numer;
}
#elif defined(_POSIX_MONOTONIC_CLOCK)
inline tick_t tick()
{
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (tick_t)t.tv_sec * 1000 * 1000 * 1000 + (tick_t)t.tv_nsec;
}
tick_t ticks_per_sec()
{
	return 1000 * 1000 * 1000;
}
#elif defined(_POSIX_C_SOURCE)
inline tick_t tick()
{
	timeval t;
	gettimeofday(&t, NULL);
	return (tick_t)t.tv_sec * 1000 * 1000 + (tick_t)t.tv_usec;
}
tick_t ticks_per_sec()
{
	return 1000 * 1000;
}
#elif defined(WIN32)
inline tick_t tick()
{
	LARGE_INTEGER ddwTimer;
	QueryPerformanceCounter(&ddwTimer);
	return ddwTimer.QuadPart;
}
tick_t ticks_per_sec()
{
	LARGE_INTEGER ddwTimerFrequency;
	QueryPerformanceFrequency(&ddwTimerFrequency);
	return ddwTimerFrequency.QuadPart;
}
#else
#error No tick_t
#endif

const int N = 2*1024*1024;

tick_t scalarTotalTicks = 0;
tick_t simdTotalTicks = 0;
tick_t scalarTicks = 0;
const char *chartName = "";
#define SETCHART(x) chartName = (x);

#define START() \
	do { \
		tick_t start = tick();

bool comma=false;
#define END(result, name) \
		tick_t end = tick(); \
		tick_t ticks = end - start; \
		scalarTotalTicks += scalarTicks; \
		simdTotalTicks += ticks; \
		double nsecs = (double)ticks * 1000.0 * 1000.0 * 1000.0 / ticks_per_sec() / N; \
		printf("%s{ \"chart\": \"%s\", \"category\": \"%s\", \"scalar\": %f, \"simd\": %f }\n", comma?",":"", chartName, name, scalarTime, nsecs); \
		comma = true; \
		/*printf(name ": %f msecs (%.3fx of scalar)%s\n", msecs, msecs/scalarTime);*/ \
		printf("%s", (result) != 0 ? "Error!" : ""); \
	} while(0)

#define ENDSCALAR(result, name) \
		tick_t end = tick(); \
		scalarTicks = end - start; \
		scalarTime = (double)scalarTicks * 1000.0 * 1000.0 * 1000.0 / ticks_per_sec() / N; \
		printf("%s", (result) != 0 ? "Error!" : ""); \
	} while(0)

void Print(__m128 m)
{
	float val[4];
	_mm_storeu_ps(val, m);
	fprintf(stderr, "[%g, %g, %g, %g]\n", val[3], val[2], val[1], val[0]);
}

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#define INLINE __forceinline
#else
#define NOINLINE __attribute__((noinline))
#define INLINE __inline__
#endif

float NOINLINE *get_src() { return always_true() ? (float*)aligned_alloc(16, (N+16)*sizeof(float)) : 0; }
float NOINLINE *get_src2() { return always_true() ? (float*)aligned_alloc(16, (N+16)*sizeof(float)) : 0; }
float NOINLINE *get_dst() { return always_true() ? (float*)aligned_alloc(16, (N+16)*sizeof(float)) : 0; }

double NOINLINE *get_src_d() { return always_true() ? (double*)aligned_alloc(16, (N+16)*sizeof(double)) : 0; }
double NOINLINE *get_src2_d() { return always_true() ? (double*)aligned_alloc(16, (N+16)*sizeof(double)) : 0; }
double NOINLINE *get_dst_d() { return always_true() ? (double*)aligned_alloc(16, (N+16)*sizeof(double)) : 0; }

template<typename T>
T checksum_dst(T *dst)
{
	if (always_true()) return 0.f;
	else 
	{
		T s = 0.f; for(int i = 0; i < N; ++i) s += dst[i];
		return s;
	}
}

uint32_t fcastu(float f) { return *(uint32_t*)&f; }
uint64_t dcastu(double f) { return *(uint64_t*)&f; }
float ucastf(uint32_t t) { return *(float*)&t; }
double ucastd(uint64_t t) { return *(double*)&t; }

// load-store test
#define LS_TEST(msg, load_instr, load_offset, store_instr, store_ptr_type, store_offset, num_elems_stride) \
	START(); \
		for(int i = 0; i < N; i += num_elems_stride) \
			store_instr((store_ptr_type)dst+store_offset+i, load_instr(src+load_offset+i)); \
	END(checksum_dst(dst), msg);

// loadh/l - store test
#define LSH_TEST(msg, reg, load_instr, load_ptr_type, load_offset, store_instr, store_ptr_type, store_offset, num_elems_stride) \
	START(); \
		for(int i = 0; i < N; i += num_elems_stride) \
			store_instr((store_ptr_type)dst+store_offset+i, load_instr(reg, (load_ptr_type)(src+load_offset+i))); \
	END(checksum_dst(dst), msg);

#define LS64_TEST(msg, load_instr, load_offset, store_instr, store_offset, num_elems_stride) \
	START(); \
		for(int i = 0; i < N; i += num_elems_stride) \
			store_instr((__m64*)(dst+store_offset+i), load_instr(src+load_offset+i)); \
	END(checksum_dst(dst), msg);

// set-store test
#define SS_TEST(msg, set_instr) \
	START(); \
		for(int i = 0; i < N; i += 4) \
			_mm_store_ps((float*)dst+i, set_instr); \
	END(checksum_dst(dst), msg);

#define UNARYOP_TEST(msg, instr, op0) \
	START(); \
		__m128 o = op0; \
		for(int i = 0; i < N; i += 4) \
			o = instr(o); \
		_mm_store_ps(dst, o); \
	END(checksum_dst(dst), msg);

#define BINARYOP_TEST(msg, instr, op0, op1) \
	START(); \
		__m128 o0 = op0; \
		__m128 o1 = op1; \
		for(int i = 0; i < N; i += 4) \
			o0 = instr(o0, o1); \
		_mm_store_ps(dst, o0); \
	END(checksum_dst(dst), msg);

#define SS_TEST_D(msg, set_instr) \
	START(); \
		for(int i = 0; i < N; i += 2) \
			_mm_store_pd((double*)dst+i, set_instr); \
	END(checksum_dst(dst), msg);

#define UNARYOP_TEST_D(msg, instr, op0) \
	START(); \
		__m128d o = op0; \
		for(int i = 0; i < N; i += 2) \
			o = instr(o); \
		_mm_store_pd(dst, o); \
	END(checksum_dst(dst), msg);

#define BINARYOP_TEST_D(msg, instr, op0, op1) \
	START(); \
		__m128d o0 = op0; \
		__m128d o1 = op1; \
		for(int i = 0; i < N; i += 2) \
			o0 = instr(o0, o1); \
		_mm_store_pd(dst, o0); \
	END(checksum_dst(dst), msg);

#define Max(a,b) ((a) >= (b) ? (a) : (b))
#define Min(a,b) ((a) <= (b) ? (a) : (b))

static INLINE int Isnan(float __f)
{
  return (*(unsigned int*)&__f << 1) > 0xFF000000u;
}
