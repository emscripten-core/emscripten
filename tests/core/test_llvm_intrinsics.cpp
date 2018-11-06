// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <sys/types.h>

extern "C" {
extern unsigned short llvm_bswap_i16(unsigned short x);
extern unsigned int llvm_bswap_i32(unsigned int x);
extern int32_t llvm_ctlz_i8(int8_t x, int izZeroUndef);
extern int32_t llvm_ctlz_i16(int16_t x, int izZeroUndef);
extern int32_t llvm_ctlz_i32(int32_t x, int izZeroUndef);
extern int64_t llvm_ctlz_i64(int64_t x, int izZeroUndef);
extern int32_t llvm_cttz_i32(int32_t x, int izZeroUndef);
extern int64_t llvm_cttz_i64(int64_t x, int izZeroUndef);
extern int32_t llvm_ctpop_i32(int32_t x);
extern int64_t llvm_ctpop_i64(int64_t x);
extern int llvm_expect_i32(int x, int y);
extern float llvm_powi_f32(float x, int32_t y);
extern double llvm_powi_f64(double x, int32_t y);
extern float llvm_trunc_f32(float x);
extern double llvm_trunc_f64(double x);
extern float llvm_ceil_f32(float x);
extern double llvm_ceil_f64(double x);
extern float llvm_floor_f32(float x);
extern double llvm_floor_f64(double x);
extern float llvm_cos_f32(float x);
extern double llvm_cos_f64(double x);
extern float llvm_sin_f32(float x);
extern double llvm_sin_f64(double x);
extern float llvm_exp2_f32(float x);
extern double llvm_exp2_f64(double x);
extern float llvm_log2_f32(float x);
extern double llvm_log2_f64(double x);
extern float llvm_log10_f32(float x);
extern double llvm_log10_f64(double x);

extern float llvm_copysign_f32(float x, float y);
extern double llvm_copysign_f64(double x, double y);

extern float llvm_round_f32(float x);
extern double llvm_round_f64(double x);
extern float llvm_minnum_f32(float x, float y);
extern double llvm_minnum_f64(double x, double y);
extern float llvm_maxnum_f32(float x, float y);
extern double llvm_maxnum_f64(double x, double y);
extern float llvm_nearbyint_f32(float x);
extern double llvm_nearbyint_f64(double x);
}

int main(void) {
  unsigned short x = 0xc8ef;
  printf("%x,%x\n", x & 0xff, x >> 8);
  x = llvm_bswap_i16(x);
  printf("%x,%x\n", x & 0xff, x >> 8);

  unsigned int y = 0xc5de158a;
  printf("%x,%x,%x,%x\n", y & 0xff, (y >> 8) & 0xff, (y >> 16) & 0xff,
         (y >> 24) & 0xff);
  y = llvm_bswap_i32(y);
  printf("%x,%x,%x,%x\n", y & 0xff, (y >> 8) & 0xff, (y >> 16) & 0xff,
         (y >> 24) & 0xff);

  printf("%d,%d\n", (int)llvm_ctlz_i64(((int64_t)1) << 40, 0),
         llvm_ctlz_i32(1 << 10, 0));
  printf("%d,%d\n", (int)llvm_cttz_i64(((int64_t)1) << 40, 0),
         llvm_cttz_i32(1 << 10, 0));
  printf("%d,%d\n", (int)llvm_ctpop_i64((0x3101ULL << 32) | 1),
         llvm_ctpop_i32(0x3101));

  printf("llvm_cttz_i32:\n");
  printf("(0, 0)=%d\n", llvm_cttz_i32(0, 0));
  printf("(1, 0)=%d\n", llvm_cttz_i32(1, 0));
  printf("(2, 0)=%d\n", llvm_cttz_i32(2, 0));
  printf("(0x0000FFFF, 0)=%d\n", llvm_cttz_i32(0x0000FFFF, 0));
  printf("(0x7FFF0000, 0)=%d\n", llvm_cttz_i32(0x7FFF0000, 0));
  printf("(0xFFFF0000, 0)=%d\n", llvm_cttz_i32(0xFFFF0000, 0));
  printf("(0x7FFFFFFF, 0)=%d\n", llvm_cttz_i32(0x7FFFFFFF, 0));
  printf("(0xFFFFFFFE, 0)=%d\n", llvm_cttz_i32(0xFFFFFFFE, 0));
  printf("(0xFFFFFFFF, 0)=%d\n", llvm_cttz_i32(0xFFFFFFFF, 0));
  printf("small ctlz: %d,%d\n", (int)llvm_ctlz_i8(2, 0), llvm_ctlz_i16(2, 0));

  printf("llvm_ctpop_i32:\n");
  printf("%d\n", (int)llvm_ctpop_i32(-594093059)); // 22
  printf("%d\n", (int)llvm_ctpop_i32(0xdeadbeef)); // 24
  printf("%d\n", (int)llvm_ctpop_i32(0x00000000)); // 0
  printf("%d\n", (int)llvm_ctpop_i32(0xffffffff)); // 32
  printf("%d\n", (int)llvm_ctpop_i32(0x55555555)); // 16
  printf("%d\n", (int)llvm_ctpop_i32(0xa55a5aa5)); // 16
  printf("%d\n", (int)llvm_ctpop_i32(0xaaaaaaaa)); // 16
  printf("%d\n", (int)llvm_ctpop_i32(0x80000000)); // 1
  printf("%d\n", (int)llvm_ctpop_i32(0x00000001)); // 1
  printf("llvm_expect_i32:\n");
  printf("%d\n", llvm_expect_i32(x % 27, 3));

  int64_t a = 1;
  a = __builtin_bswap64(a);
  printf("%lld\n", a);

  printf("%d\n", (int)llvm_powi_f32(5.0f, 3));
  printf("%d\n", (int)llvm_powi_f64(3.0, 5));
  printf("%d\n", (int)llvm_trunc_f32(18.0987f));
  printf("%d\n", (int)llvm_trunc_f64(-12.42));
  printf("%d\n", (int)llvm_floor_f32(27.665f));
  printf("%d\n", (int)llvm_floor_f64(-8.95));
  printf("%.1f\n", llvm_cos_f32(0.0f * 3.14/180));
  printf("%.1f\n", llvm_cos_f64(180.0 * 3.14/180));
  printf("%.1f\n", llvm_sin_f32(90.0f * 3.14/180));
  printf("%.1f\n", llvm_sin_f64(270.0 * 3.14/180));

  printf("exp2_f32 %.1f\n", llvm_exp2_f32(3));
  printf("exp2_f64 %.1f\n", llvm_exp2_f64(4.5));
  printf("log2_f32 %.1f\n", llvm_log2_f32(16));
  printf("log2_f64 %.1f\n", llvm_log2_f64(20));
  printf("log10_f32 %.1f\n", llvm_log10_f32(1000));
  printf("log10_f64 %.1f\n", llvm_log10_f64(2000));

  printf("llvm_ceil_f32 %.1f\n", llvm_ceil_f32(1.4f));
  printf("llvm_ceil_f32 %.1f\n", llvm_ceil_f32(1.5f));
  printf("llvm_ceil_f32 %.1f\n", llvm_ceil_f32(1.6f));
  printf("llvm_ceil_f32 %.1f\n", llvm_ceil_f32(-1.4f));
  printf("llvm_ceil_f32 %.1f\n", llvm_ceil_f32(-1.5f));
  printf("llvm_ceil_f32 %.1f\n", llvm_ceil_f32(-1.6f));

  printf("llvm_ceil_f64 %.1f\n", llvm_ceil_f64(1.4));
  printf("llvm_ceil_f64 %.1f\n", llvm_ceil_f64(1.5));
  printf("llvm_ceil_f64 %.1f\n", llvm_ceil_f64(1.6));
  printf("llvm_ceil_f64 %.1f\n", llvm_ceil_f64(-1.4));
  printf("llvm_ceil_f64 %.1f\n", llvm_ceil_f64(-1.5));
  printf("llvm_ceil_f64 %.1f\n", llvm_ceil_f64(-1.6));

  printf("llvm_floor_f32 %.1f\n", llvm_floor_f32(1.4f));
  printf("llvm_floor_f32 %.1f\n", llvm_floor_f32(1.5f));
  printf("llvm_floor_f32 %.1f\n", llvm_floor_f32(1.6f));
  printf("llvm_floor_f32 %.1f\n", llvm_floor_f32(-1.4f));
  printf("llvm_floor_f32 %.1f\n", llvm_floor_f32(-1.5f));
  printf("llvm_floor_f32 %.1f\n", llvm_floor_f32(-1.6f));

  printf("llvm_floor_f64 %.1f\n", llvm_floor_f64(1.4));
  printf("llvm_floor_f64 %.1f\n", llvm_floor_f64(1.5));
  printf("llvm_floor_f64 %.1f\n", llvm_floor_f64(1.6));
  printf("llvm_floor_f64 %.1f\n", llvm_floor_f64(-1.4));
  printf("llvm_floor_f64 %.1f\n", llvm_floor_f64(-1.5));
  printf("llvm_floor_f64 %.1f\n", llvm_floor_f64(-1.6));

  printf("llvm_round_f64 %.1f\n", llvm_round_f64(20.49));
  printf("llvm_round_f64 %.1f\n", llvm_round_f64(20.5));
  printf("llvm_round_f64 %.1f\n", llvm_round_f64(42));
  printf("llvm_round_f64 %.1f\n", llvm_round_f64(-20.49));
  printf("llvm_round_f64 %.1f\n", llvm_round_f64(-20.5));
  printf("llvm_round_f64 %.1f\n", llvm_round_f64(-20.51));

  printf("llvm_round_f32 %.1f\n", llvm_round_f32(20.49));
  printf("llvm_round_f32 %.1f\n", llvm_round_f32(20.5));
  printf("llvm_round_f32 %.1f\n", llvm_round_f32(42));
  printf("llvm_round_f32 %.1f\n", llvm_round_f32(-20.49));
  printf("llvm_round_f32 %.1f\n", llvm_round_f32(-20.5));
  printf("llvm_round_f32 %.1f\n", llvm_round_f32(-20.51));

  printf("llvm_nearbyint_f64 %.1f\n", llvm_nearbyint_f64(20.50));
  printf("llvm_nearbyint_f64 %.1f\n", llvm_nearbyint_f64(20.51));
  printf("llvm_nearbyint_f64 %.1f\n", llvm_nearbyint_f64(42));
  printf("llvm_nearbyint_f64 %.1f\n", llvm_nearbyint_f64(-20.49));
  printf("llvm_nearbyint_f64 %.1f\n", llvm_nearbyint_f64(-20.5));
  printf("llvm_nearbyint_f64 %.1f\n", llvm_nearbyint_f64(-20.51));

  printf("llvm_nearbyint_f32 %.1f\n", llvm_nearbyint_f32(20.50));
  printf("llvm_nearbyint_f32 %.1f\n", llvm_nearbyint_f32(20.51));
  printf("llvm_nearbyint_f32 %.1f\n", llvm_nearbyint_f32(42));
  printf("llvm_nearbyint_f32 %.1f\n", llvm_nearbyint_f32(-20.49));
  printf("llvm_nearbyint_f32 %.1f\n", llvm_nearbyint_f32(-20.5));
  printf("llvm_nearbyint_f32 %.1f\n", llvm_nearbyint_f32(-20.51));

  printf("llvm_minnum_f32 %.1f\n", llvm_minnum_f32(5.7, 10.2));
  printf("llvm_minnum_f32 %.1f\n", llvm_minnum_f32(8.5, 2.3));
  printf("llvm_minnum_f64 %.1f\n", llvm_minnum_f64(5.7, 10.2));
  printf("llvm_minnum_f64 %.1f\n", llvm_minnum_f64(8.5, 2.3));

  printf("llvm_maxnum_f32 %.1f\n", llvm_maxnum_f32(5.7, 10.2));
  printf("llvm_maxnum_f32 %.1f\n", llvm_maxnum_f32(8.5, 2.3));
  printf("llvm_maxnum_f64 %.1f\n", llvm_maxnum_f64(5.7, 10.2));
  printf("llvm_maxnum_f64 %.1f\n", llvm_maxnum_f64(8.5, 2.3));

  printf("llvm_copysign_f32 %.1f\n", llvm_copysign_f32(-1.2, 3.4));
  printf("llvm_copysign_f32 %.1f\n", llvm_copysign_f32(5.6, -7.8));
  printf("llvm_copysign_f32 %.1f\n", llvm_copysign_f32(-1.3, -2.4));
  printf("llvm_copysign_f32 %.1f\n", llvm_copysign_f32(5.7, 6.8));
  printf("llvm_copysign_f64 %.1f\n", llvm_copysign_f64(-1.2, 3.4));
  printf("llvm_copysign_f64 %.1f\n", llvm_copysign_f64(5.6, -7.8));
  printf("llvm_copysign_f64 %.1f\n", llvm_copysign_f64(-1.3, -2.4));
  printf("llvm_copysign_f64 %.1f\n", llvm_copysign_f64(5.7, 6.8));

  printf("ok.\n");

  return 0;
}
