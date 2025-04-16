#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#if defined(__unix__) && !defined(__EMSCRIPTEN__) // Native build without Emscripten.
#include <time.h>
#include <errno.h>
#include <string.h>
#endif

#ifdef __APPLE__
#define aligned_alloc(align, size) malloc((size))
#endif

#ifdef WIN32
#include <Windows.h>
#define aligned_alloc(align, size) _aligned_malloc((size), (align))
#endif

// Scalar horizonal max across four lanes.
float hmax(__m128 m) {
  float f[4];
  _mm_storeu_ps(f, m);
  return fmax(fmax(f[0], f[1]), fmax(f[2], f[3]));
}

#include "../tick.h"

const int N = 8*1024*1024;

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

// Slightly awkward way to allocate so that compiler will definitely not see this memory area as compile-time optimizable:
int NOINLINE *alloc_int_buffer() { return always_true() ? (int*)aligned_alloc(16, (N+16)*sizeof(int)) : 0; }
float NOINLINE *alloc_float_buffer() { return always_true() ? (float*)aligned_alloc(16, (N+16)*sizeof(float)) : 0; }
double NOINLINE *alloc_double_buffer() { return always_true() ? (double*)aligned_alloc(16, (N+16)*sizeof(double)) : 0; }

template<typename T>
T checksum_dst(T *dst) {
  if (always_true()) {
    return 0.f;
  } else {
    T s = 0.f; for(int i = 0; i < N; ++i) s += dst[i];
    return s;
  }
}

uint32_t fcastu(float f) { return *(uint32_t*)&f; }
uint64_t dcastu(double f) { return *(uint64_t*)&f; }
float ucastf(uint32_t t) { return *(float*)&t; }
double ucastd(uint64_t t) { return *(double*)&t; }

#define LOAD_STORE_F(msg, load_instr, load_offset, store_instr, store_ptr_type, store_offset, num_elems_stride) \
  START(); \
    for(int i = 0; i < N; i += num_elems_stride) \
      store_instr((store_ptr_type)dst_flt+store_offset+i, load_instr(src_flt+load_offset+i)); \
  END(checksum_dst(dst_flt), msg);

#define LOAD_STORE_D(msg, load_instr, load_offset, store_instr, store_ptr_type, store_offset, num_elems_stride) \
  START(); \
    for(int i = 0; i < N; i += num_elems_stride) \
      store_instr((store_ptr_type)dst_dbl+store_offset+i, load_instr(src_dbl+load_offset+i)); \
  END(checksum_dst(dst_dbl), msg);

#define LOAD_STORE_I(msg, load_instr, load_offset, store_instr, store_offset, num_elems_stride) \
  START(); \
    for(int i = 0; i < N; i += num_elems_stride) \
      store_instr((__m128i*)(dst_int+store_offset+i), load_instr((__m128i*)(src_int+load_offset+i))); \
  END(checksum_dst(dst_int), msg);

// load M64*, store M128
#define LOAD_STORE_M64(msg, reg, load_instr, load_ptr_type, load_offset, store_instr, store_ptr_type, store_offset, num_elems_stride) \
  START(); \
    for(int i = 0; i < N; i += num_elems_stride) \
      store_instr((store_ptr_type)dst_flt+store_offset+i, load_instr(reg, (load_ptr_type)(src_flt+load_offset+i))); \
  END(checksum_dst(dst_flt), msg);

#define LOAD_STORE_64_F(msg, load_instr, load_offset, store_instr, store_offset, num_elems_stride) \
  START(); \
    for(int i = 0; i < N; i += num_elems_stride) \
      store_instr((__m64*)(dst_flt+store_offset+i), load_instr(src_flt+load_offset+i)); \
  END(checksum_dst(dst_flt), msg);

#define LOAD_STORE_64_D(msg, load_instr, load_offset, store_instr, store_offset, num_elems_stride) \
  START(); \
    for(int i = 0; i < N; i += num_elems_stride) \
      store_instr((__m64*)(dst_dbl+store_offset+i), load_instr(src_dbl+load_offset+i)); \
  END(checksum_dst(dst_dbl), msg);

#define SET_STORE_F(msg, set_instr) \
  START(); \
    for(int i = 0; i < N; i += 4) \
      _mm_store_ps(dst_flt+i, set_instr); \
  END(checksum_dst(dst_flt), msg);

#define SET_STORE_D(msg, set_instr) \
  START(); \
    for(int i = 0; i < N; i += 4) \
      _mm_store_pd(dst_dbl+i, set_instr); \
  END(checksum_dst(dst_dbl), msg);

#define UNARYOP_F_F(msg, instr, op0) \
  START(); \
    __m128 o = op0; \
    for(int i = 0; i < N; i += 4) \
      o = instr(o); \
    _mm_store_ps(dst_flt, o); \
  END(checksum_dst(dst_flt), msg);

#define UNARYOP_I_I(msg, instr, op0) \
  START(); \
    __m128 o = op0; \
    for(int i = 0; i < N; i += 4) \
      o = instr(o); \
    _mm_store_si128((__m128i*)dst_int, o); \
  END(checksum_dst(dst_int), msg);

#define UNARYOP_i_F(msg, instr) \
  START(); \
    for(int i = 0; i < N; i += 4) \
      dst_int_scalar += instr; \
  END(dst_int_scalar, msg);

#define UNARYOP_D_D(msg, instr, op0) \
  START(); \
    __m128d o = op0; \
    for(int i = 0; i < N; i += 2) \
      o = instr(o); \
    _mm_store_pd(dst_dbl, o); \
  END(checksum_dst(dst_dbl), msg);

#define BINARYOP_F_FF(msg, instr, op0, op1) \
  START(); \
    __m128 o0 = op0; \
    __m128 o1 = op1; \
    for(int i = 0; i < N; i += 4) \
      o0 = instr(o0, o1); \
    _mm_store_ps(dst_flt, o0); \
  END(checksum_dst(dst_flt), msg);

#define BINARYOP_I_II(msg, instr, op0, op1) \
  START(); \
    __m128 o0 = op0; \
    __m128 o1 = op1; \
    for(int i = 0; i < N; i += 4) \
      o0 = instr(o0, o1); \
    _mm_store_si128((__m128i*)dst_int, o0); \
  END(checksum_dst(dst_int), msg);

#define BINARYOP_D_DD(msg, instr, op0, op1) \
  START(); \
    __m128d o0 = op0; \
    __m128d o1 = op1; \
    for(int i = 0; i < N; i += 2) \
      o0 = instr(o0, o1); \
    _mm_store_pd(dst_dbl, o0); \
  END(checksum_dst(dst_dbl), msg);

#define Max(a,b) ((a) >= (b) ? (a) : (b))
#define Min(a,b) ((a) <= (b) ? (a) : (b))

static INLINE int Isnan(float __f) {
  return (*(unsigned int*)&__f << 1) > 0xFF000000u;
}
