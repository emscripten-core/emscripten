#include <xmmintrin.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define _mm_storeu_ps _mm_store_ps // Hack for missing function. Works for now since Emscripten does not care about alignment.
#define aligned_alloc(align, size) malloc(size) // Hack for missing function. Works for now since Emscripten does not care about alignment.
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#define aligned_alloc(align, size) malloc(size)
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
#else
#error No tick_t
#endif

#define START() do { tick_t start = tick();
#define END(result, name) tick_t end = tick(); double msecs = (double)(end - start) / ticks_per_sec(); printf("Block " name " took %f msecs (%.3fx of scalar) Result: %f.\n", msecs, msecs/scalarTime, (result)); } while(0)
#define ENDSCALAR(result, name) tick_t end = tick(); scalarTime = (double)(end - start) / ticks_per_sec(); printf("Block " name " took %f msecs. Result: %f.\n", scalarTime, (result)); } while(0)

void Print(__m128 m)
{
	float val[4];
	_mm_storeu_ps(val, m);
	fprintf(stderr, "[%g, %g, %g, %g]\n", val[3], val[2], val[1], val[0]);
}

int main()
{
	const int N = 16*1024*1024;
	printf("N: %u\n", N);
	float *src = (float*)aligned_alloc(16, N*sizeof(float));
	assert(N%4 == 0); // Don't care about the tail for now.
	for(int i = 0; i < N; ++i)
		src[i] = (float)rand() / RAND_MAX;

	float scalarTime;

	// Scalar
	float mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; ++f)
			mx = (mx >= *f) ? mx : *f;
	ENDSCALAR(mx, "scalar");

	// Scalar 4 unroll
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 4)
		{
			mx = (mx >= f[0]) ? mx : f[0];
			mx = (mx >= f[1]) ? mx : f[1];
			mx = (mx >= f[2]) ? mx : f[2];
			mx = (mx >= f[3]) ? mx : f[3];
		}
	END(mx, "scalar 4 unroll");

	// SSE1 No unroll
	__m128 m = _mm_set1_ps(-INFINITY);
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 4)
			m = _mm_max_ps(_mm_load_ps(f), m);
		mx = hmax(m);
	END(mx, "SSE1 no unroll");

	// SSE1 Unroll 2
	m = _mm_set1_ps(-INFINITY);
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 8)
		{
			m = _mm_max_ps(_mm_load_ps(f), m);
			m = _mm_max_ps(_mm_load_ps(f+4), m);
		}
		mx = hmax(m);
	END(mx, "SSE1 Unroll 2");

	// SSE1 Unroll 4
	m = _mm_set1_ps(-INFINITY);
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 16)
		{
			m = _mm_max_ps(_mm_load_ps(f), m);
			m = _mm_max_ps(_mm_load_ps(f+4), m);
			m = _mm_max_ps(_mm_load_ps(f+8), m);
			m = _mm_max_ps(_mm_load_ps(f+12), m);
		}
		mx = hmax(m);
	END(mx, "SSE1 Unroll 4");

#ifndef __EMSCRIPTEN__
	// SSE1 Unroll 4 prefetch
	m = _mm_set1_ps(-INFINITY);
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 16)
		{
			_mm_prefetch(f + 64, _MM_HINT_T0);
			m = _mm_max_ps(_mm_load_ps(f), m);
			m = _mm_max_ps(_mm_load_ps(f+4), m);
			m = _mm_max_ps(_mm_load_ps(f+8), m);
			m = _mm_max_ps(_mm_load_ps(f+12), m);
		}
		mx = hmax(m);
	END(mx, "SSE1 Unroll 4 pf");
#endif

	// SSE1 Unroll 16
	m = _mm_set1_ps(-INFINITY);
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 64)
		{
			m = _mm_max_ps(_mm_load_ps(f), m);
			m = _mm_max_ps(_mm_load_ps(f+4), m);
			m = _mm_max_ps(_mm_load_ps(f+8), m);
			m = _mm_max_ps(_mm_load_ps(f+12), m);
			m = _mm_max_ps(_mm_load_ps(f+16), m);
			m = _mm_max_ps(_mm_load_ps(f+20), m);
			m = _mm_max_ps(_mm_load_ps(f+24), m);
			m = _mm_max_ps(_mm_load_ps(f+28), m);
			m = _mm_max_ps(_mm_load_ps(f+32), m);
			m = _mm_max_ps(_mm_load_ps(f+36), m);
			m = _mm_max_ps(_mm_load_ps(f+40), m);
			m = _mm_max_ps(_mm_load_ps(f+44), m);
			m = _mm_max_ps(_mm_load_ps(f+48), m);
			m = _mm_max_ps(_mm_load_ps(f+52), m);
			m = _mm_max_ps(_mm_load_ps(f+56), m);
			m = _mm_max_ps(_mm_load_ps(f+60), m);
		}
		mx = hmax(m);
	END(mx, "SSE1 Unroll 16");

#ifndef __EMSCRIPTEN__
	// SSE1 Unroll 16 prefetch
	m = _mm_set1_ps(-INFINITY);
	mx = -INFINITY;
	START();
		for(float *f = src; f < src+N; f += 64)
		{
			_mm_prefetch(f + 64, _MM_HINT_T0);
			_mm_prefetch(f + 80, _MM_HINT_T0);
			_mm_prefetch(f + 96, _MM_HINT_T0);
			_mm_prefetch(f + 112, _MM_HINT_T0);
			m = _mm_max_ps(_mm_load_ps(f), m);
			m = _mm_max_ps(_mm_load_ps(f+4), m);
			m = _mm_max_ps(_mm_load_ps(f+8), m);
			m = _mm_max_ps(_mm_load_ps(f+12), m);
			m = _mm_max_ps(_mm_load_ps(f+16), m);
			m = _mm_max_ps(_mm_load_ps(f+20), m);
			m = _mm_max_ps(_mm_load_ps(f+24), m);
			m = _mm_max_ps(_mm_load_ps(f+28), m);
			m = _mm_max_ps(_mm_load_ps(f+32), m);
			m = _mm_max_ps(_mm_load_ps(f+36), m);
			m = _mm_max_ps(_mm_load_ps(f+40), m);
			m = _mm_max_ps(_mm_load_ps(f+44), m);
			m = _mm_max_ps(_mm_load_ps(f+48), m);
			m = _mm_max_ps(_mm_load_ps(f+52), m);
			m = _mm_max_ps(_mm_load_ps(f+56), m);
			m = _mm_max_ps(_mm_load_ps(f+60), m);
		}
		mx = hmax(m);
	END(mx, "SSE1 Unroll 16 pf");
#endif
}
