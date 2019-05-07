#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <emscripten.h>
#include <simd128.h>

#define TESTFN EMSCRIPTEN_KEEPALIVE __attribute__((noinline))

i8x16 TESTFN i8x16_load(i8x16 *ptr) {
  return (i8x16) wasm_v128_load(ptr);
}
void TESTFN i8x16_store(i8x16 *ptr, i8x16 vec) {
  wasm_v128_store(ptr, vec);
}
v128 TESTFN i32x4_const(void) {
  return wasm_v128_const(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
}
i8x16 TESTFN i8x16_shuffle_interleave_bytes(i8x16 x, i8x16 y) {
  return __builtin_shufflevector(x, y, 0, 17, 2, 19, 4, 21, 6, 23, 8, 25, 10, 27, 12, 29, 14, 31);
}
i32x4 TESTFN i32x4_shuffle_reverse(i32x4 vec) {
  return __builtin_shufflevector(vec, vec, 3, 2, 1, 0);
}
i8x16 TESTFN i8x16_splat(int32_t x) {
  return wasm_i8x16_splat(x);
}
int32_t TESTFN i8x16_extract_lane_s_first(i8x16 vec) {
      return wasm_i8x16_extract_lane(vec, 0);
}
int32_t TESTFN i8x16_extract_lane_s_last(i8x16 vec) {
        return wasm_i8x16_extract_lane(vec, 15);
}
#ifdef __wasm_unimplemented_simd128__
uint32_t TESTFN i8x16_extract_lane_u_first(i8x16 vec) {
  return wasm_u8x16_extract_lane(vec, 0);
}
uint32_t TESTFN i8x16_extract_lane_u_last(i8x16 vec) {
  return wasm_u8x16_extract_lane(vec, 15);
}
#endif // __wasm_unimplemented_simd128__
i8x16 TESTFN i8x16_replace_lane_first(i8x16 vec, int32_t val) {
  return wasm_i8x16_replace_lane(vec, 0, val);
}
i8x16 TESTFN i8x16_replace_lane_last(i8x16 vec, int32_t val) {
  return wasm_i8x16_replace_lane(vec, 15, val);
}
i16x8 TESTFN i16x8_splat(int32_t x) {
  return wasm_i16x8_splat(x);
}
int32_t TESTFN i16x8_extract_lane_s_first(i16x8 vec) {
  return wasm_i16x8_extract_lane(vec, 0);
}
int32_t TESTFN i16x8_extract_lane_s_last(i16x8 vec) {
  return wasm_i16x8_extract_lane(vec, 7);
}
#ifdef __wasm_unimplemented_simd128__
int32_t TESTFN i16x8_extract_lane_u_first(i16x8 vec) {
    return wasm_u16x8_extract_lane(vec, 0);
}
int32_t TESTFN i16x8_extract_lane_u_last(i16x8 vec) {
  return wasm_u16x8_extract_lane(vec, 7);
}
#endif // __wasm_unimplemented_simd128__
i16x8 TESTFN i16x8_replace_lane_first(i16x8 vec, int32_t val) {
  return wasm_i16x8_replace_lane(vec, 0, val);
}
i16x8 TESTFN i16x8_replace_lane_last(i16x8 vec, int32_t val) {
  return wasm_i16x8_replace_lane(vec, 7, val);
}
i32x4 TESTFN i32x4_splat(int32_t x) {
  return wasm_i32x4_splat(x);
}
int32_t TESTFN i32x4_extract_lane_first(i32x4 vec) {
  return wasm_i32x4_extract_lane(vec, 0);
}
int32_t TESTFN i32x4_extract_lane_last(i32x4 vec) {
  return wasm_i32x4_extract_lane(vec, 3);
}
i32x4 TESTFN i32x4_replace_lane_first(i32x4 vec, int32_t val) {
  return wasm_i32x4_replace_lane(vec, 0, val);
}
i32x4 TESTFN i32x4_replace_lane_last(i32x4 vec, int32_t val) {
  return wasm_i32x4_replace_lane(vec, 3, val);
}
i64x2 TESTFN i64x2_splat(int64_t x) {
  return wasm_i64x2_splat(x);
}
#ifdef __wasm_unimplemented_simd128__
int64_t TESTFN i64x2_extract_lane_first(i64x2 vec) {
  return wasm_i64x2_extract_lane(vec, 0);
}
int64_t TESTFN i64x2_extract_lane_last(i64x2 vec) {
  return wasm_i64x2_extract_lane(vec, 1);
}
i64x2 TESTFN i64x2_replace_lane_first(i64x2 vec, int64_t val) {
  return wasm_i64x2_replace_lane(vec, 0, val);
}
i64x2 TESTFN i64x2_replace_lane_last(i64x2 vec, int64_t val) {
  return wasm_i64x2_replace_lane(vec, 1, val);
}
#endif // __wasm_unimplemented_simd128__
f32x4 TESTFN f32x4_splat(float x) {
  return wasm_f32x4_splat(x);
}
float TESTFN f32x4_extract_lane_first(f32x4 vec) {
  return wasm_f32x4_extract_lane(vec, 0);
}
float TESTFN f32x4_extract_lane_last(f32x4 vec) {
  return wasm_f32x4_extract_lane(vec, 3);
}
f32x4 TESTFN f32x4_replace_lane_first(f32x4 vec, float val) {
  return wasm_f32x4_replace_lane(vec, 0, val);
}
f32x4 TESTFN f32x4_replace_lane_last(f32x4 vec, float val) {
  return wasm_f32x4_replace_lane(vec, 3, val);
}
#ifdef __wasm_unimplemented_simd128__
f64x2 TESTFN f64x2_splat(int64_t x) {
  return wasm_f64x2_splat((double ) x);
}
double TESTFN f64x2_extract_lane_first(f64x2 vec) {
    return wasm_f64x2_extract_lane(vec, 0);
}
double TESTFN f64x2_extract_lane_last(f64x2 vec) {
  return wasm_f64x2_extract_lane(vec, 1);
}
f64x2 TESTFN f64x2_replace_lane_first(f64x2 vec, double val) {
  return wasm_f64x2_replace_lane(vec, 0, val);
}
f64x2 TESTFN f64x2_replace_lane_last(f64x2 vec, double val) {
  return wasm_f64x2_replace_lane(vec, 1, val);
}
#endif // __wasm_unimplemented_simd128__
u8x16 TESTFN i8x16_eq(i8x16 x, i8x16 y) {
  return wasm_i8x16_eq(x, y);
}
u8x16 TESTFN i8x16_ne(i8x16 x, i8x16 y) {
  return wasm_i8x16_ne(x, y);
}
u8x16 TESTFN i8x16_lt_s(i8x16 x, i8x16 y) {
  return wasm_i8x16_lt(x, y);
}
u8x16 TESTFN i8x16_lt_u(u8x16 x, u8x16 y) {
  return wasm_u8x16_lt(x, y);
}
u8x16 TESTFN i8x16_gt_s(i8x16 x, i8x16 y) {
  return wasm_i8x16_gt(x, y);
}
u8x16 TESTFN i8x16_gt_u(u8x16 x, u8x16 y) {
  return wasm_u8x16_gt(x,y);
}
u8x16 TESTFN i8x16_le_s(i8x16 x, i8x16 y) {
  return wasm_i8x16_le(x,y);
}
u8x16 TESTFN u8x16_le_u(u8x16 x, u8x16 y) {
   return wasm_u8x16_le(x, y);
}
u8x16 TESTFN i8x16_ge_s(i8x16 x, i8x16 y) {
  return wasm_i8x16_ge(x, y);
}
u8x16  TESTFN i8x16_ge_u(u8x16 x, u8x16 y) {
  return wasm_u8x16_ge(x, y);
}
u16x8 TESTFN i16x8_eq(i16x8 x, i16x8 y) {
  return wasm_i16x8_eq(x,y);
}
u16x8 TESTFN i16x8_ne(i16x8 x, i16x8 y) {
  return wasm_i16x8_ne(x,y);
}
u16x8 TESTFN i16x8_lt_s(i16x8 x, i16x8 y) {
  return wasm_i16x8_lt(x,y);
}
u16x8 TESTFN i16x8_lt_u(u16x8 x, u16x8 y) {
    return wasm_u16x8_lt(x,y);
}
u16x8 TESTFN i16x8_gt_s(i16x8 x, i16x8 y) {
  return wasm_i16x8_gt(x,y);
}
u16x8 TESTFN i16x8_gt_u(u16x8 x, u16x8 y) {
  return wasm_u16x8_gt(x,y);
}
u16x8 TESTFN i16x8_le_s(i16x8 x, i16x8 y) {
  return wasm_i16x8_le(x, y);
}
u16x8 TESTFN i16x8_le_u(u16x8 x, u16x8 y) {
  return wasm_u16x8_le(x, y);
}
u16x8 TESTFN i16x8_ge_s(i16x8 x, i16x8 y) {
  return wasm_i16x8_ge(x, y);
}
u16x8  TESTFN i16x8_ge_u(u16x8 x, u16x8 y) {
  return wasm_u16x8_ge(x, y);
}
u32x4 TESTFN i32x4_eq(i32x4 x, i32x4 y) {
  return wasm_i32x4_eq(x, y);
}
u32x4 TESTFN i32x4_ne(i32x4 x, i32x4 y) {
  return wasm_i32x4_ne(x, y);
}
u32x4 TESTFN i32x4_lt_s(i32x4 x, i32x4 y) {
  return wasm_i32x4_lt(x, y);
}
u32x4 TESTFN i32x4_lt_u(u32x4 x, u32x4 y) {
  return wasm_u32x4_lt(x, y);
}
u32x4 TESTFN i32x4_gt_s(i32x4 x, i32x4 y) {
  return wasm_i32x4_gt(x, y);
}
u32x4 TESTFN i32x4_gt_u(u32x4 x, u32x4 y) {
  return wasm_u32x4_gt(x, y);
}
u32x4 TESTFN i32x4_le_s(i32x4 x, i32x4 y) {
  return wasm_i32x4_le(x, y);
}
u32x4 TESTFN i32x4_le_u(u32x4 x, u32x4 y) {
  return wasm_u32x4_le(x, y);
}
u32x4 TESTFN i32x4_ge_s(i32x4 x, i32x4 y) {
  return wasm_i32x4_ge(x, y);
}
u32x4  TESTFN i32x4_ge_u(u32x4 x, u32x4 y) {
  return wasm_u32x4_ge(x, y);
}
u32x4 TESTFN f32x4_eq(f32x4 x, f32x4 y) {
  return wasm_f32x4_eq(x,y);
}
u32x4 TESTFN f32x4_ne(f32x4 x, f32x4 y) {
  return wasm_f32x4_ne(x, y);
}
u32x4 TESTFN f32x4_lt(f32x4 x, f32x4 y) {
  return wasm_f32x4_lt(x, y);
}
u32x4 TESTFN f32x4_gt(f32x4 x, f32x4 y) {
  return wasm_f32x4_gt(x,y);
}
u32x4 TESTFN f32x4_le(f32x4 x, f32x4 y) {
  return wasm_f32x4_le(x, y);
}
u32x4 TESTFN f32x4_ge(f32x4 x, f32x4 y) {
  return wasm_f32x4_ge(x, y);
}
#ifdef __wasm_undefined_simd128__
u64x2 TESTFN f64x2_eq(f64x2 x, f64x2 y) {
  return wasm_f64x2_eq(x,y);
}
u64x2 TESTFN f64x2_ne(f64x2 x, f64x2 y) {
  return wasm_f64x2_ne(x,y);
}
u64x2 TESTFN f64x2_lt(f64x2 x, f64x2 y) {
  return wasm_f64x2_lt(x,y);
}
u64x2 TESTFN f64x2_gt(f64x2 x, f64x2 y) {
  return wasm_f64x2_gt(x, y);
}
u64x2 TESTFN f64x2_le(f64x2 x, f64x2 y) {
  return wasm_f64x2_le(x, y);
}
u64x2 TESTFN f64x2_ge(f64x2 x, f64x2 y) {
  return wasm_f64x2_ge(x, y);
}
#endif // __wasm_undefined_simd128__
v128 TESTFN v128_not(v128 vec) {
  return wasm_v128_not(vec);
}
v128 TESTFN v128_and(v128 x, v128 y) {
  return wasm_v128_and(x, y);
}
v128 TESTFN v128_or(v128 x, v128 y) {
  return wasm_v128_or(x,y);
}
v128 TESTFN v128_xor(v128 x, v128 y) {
  return wasm_v128_xor(x,y);
}
v128 TESTFN v128_bitselect(v128 x, v128 y, v128 cond) {
  return wasm_v128_bitselect(x, y, cond);
}
i8x16 TESTFN i8x16_neg(i8x16 vec) {
  return wasm_i8x16_neg(vec);
}
int32_t TESTFN i8x16_any_true(i8x16 vec) {
  return wasm_i8x16_any_true(vec);
}
int32_t TESTFN i8x16_all_true(i8x16 vec) {
  return wasm_i8x16_all_true(vec);
}
i8x16 TESTFN i8x16_shl(i8x16 vec, int32_t shift) {
  return wasm_i8x16_shl(vec, shift);
}
i8x16 TESTFN i8x16_shr_s(i8x16 vec, int32_t shift) {
  return wasm_i8x16_shr(vec, shift);
}
u8x16 TESTFN i8x16_shr_u(u8x16 vec, int32_t shift) {
  return wasm_u8x16_shr(vec, shift);
}
i8x16 TESTFN i8x16_add(i8x16 x, i8x16 y) {
  return wasm_i8x16_add(x,y);
}
i8x16 TESTFN i8x16_add_saturate_s(i8x16 x, i8x16 y) {
  return wasm_i8x16_add_saturate(x, y);
}
u8x16 TESTFN i8x16_add_saturate_u(u8x16 x, u8x16 y) {
  return wasm_u8x16_add_saturate(x, y);
}
i8x16 TESTFN i8x16_sub(i8x16 x, i8x16 y) {
  return wasm_i8x16_sub(x,y);
}
i8x16 TESTFN i8x16_sub_saturate_s(i8x16 x, i8x16 y) {
  return wasm_i8x16_sub_saturate(x, y);
}
u8x16 TESTFN i8x16_sub_saturate_u(u8x16 x, u8x16 y) {
  return wasm_u8x16_sub_saturate(x, y);
}
i8x16 TESTFN i8x16_mul(i8x16 x, i8x16 y) {
  return wasm_i8x16_mul(x, y);
}
i16x8 TESTFN i16x8_neg(i16x8 vec) {
  return wasm_i16x8_neg(vec);
}
bool TESTFN i16x8_any_true(i16x8 vec) {
  return wasm_i16x8_any_true(vec);
}
int32_t TESTFN i16x8_all_true(i16x8 vec) {
  return wasm_i16x8_all_true(vec);
}
i16x8 TESTFN i16x8_shl(i16x8 vec, int32_t shift) {
  return wasm_i16x8_shl(vec, shift);
}
i16x8 TESTFN i16x8_shr_s(i16x8 vec, int32_t shift) {
  return wasm_i16x8_shr(vec, shift);
}
u16x8 TESTFN i16x8_shr_u(u16x8 vec, int32_t shift) {
  return wasm_u16x8_shr(vec, shift);
}
i16x8 TESTFN i16x8_add(i16x8 x, i16x8 y) {
  return wasm_i16x8_add(x, y);
}
i16x8 TESTFN i16x8_add_saturate_s(i16x8 x, i16x8 y) {
  return wasm_i16x8_add_saturate(x, y);
}
u16x8 TESTFN i16x8_add_saturate_u(u16x8 x, u16x8 y) {
  return wasm_u16x8_add_saturate(x, y);
}
i16x8 TESTFN i16x8_sub(i16x8 x, i16x8 y) {
  return wasm_i16x8_sub(x, y);
}
i16x8 TESTFN i16x8_sub_saturate_s(i16x8 x, i16x8 y) {
  return wasm_i16x8_sub_saturate(x,y);
}
u16x8 TESTFN i16x8_sub_saturate_u(u16x8 x, u16x8 y) {
  return wasm_u16x8_sub_saturate(x, y);
}
i16x8 TESTFN i16x8_mul(i16x8 x, i16x8 y) {
  return wasm_i16x8_mul(x, y);
}
i32x4 TESTFN i32x4_neg(i32x4 vec) {
  return wasm_i32x4_neg(vec);
}
int32_t TESTFN i32x4_any_true(i32x4 vec) {
  return wasm_i32x4_any_true(vec);
}
int32_t TESTFN i32x4_all_true(i32x4 vec) {
  return wasm_i32x4_all_true(vec);
}
i32x4 TESTFN i32x4_shl(i32x4 vec, int32_t shift) {
  return wasm_i32x4_shl(vec, shift);
}
i32x4 TESTFN i32x4_shr_s(i32x4 vec, int32_t shift) {
  return wasm_i32x4_shr(vec, shift);
}
u32x4 TESTFN i32x4_shr_u(u32x4 vec, int32_t shift) {
  return wasm_u32x4_shr(vec, shift);
}
i32x4 TESTFN i32x4_add(i32x4 x, i32x4 y) {
  return wasm_i32x4_add(x, y);
}
i32x4 TESTFN i32x4_sub(i32x4 x, i32x4 y) {
  return wasm_i32x4_sub(x, y);
}
i32x4 TESTFN i32x4_mul(i32x4 x, i32x4 y) {
  return wasm_i32x4_mul(x, y);
}
i64x2 TESTFN i64x2_neg(i64x2 vec) {
  return wasm_i64x2_neg(vec);
}
#ifdef __wasm_unimplemented_simd128__
bool TESTFN i64x2_any_true(i64x2 vec) {
  return wasm_i64x2_any_true(vec);
}
bool TESTFN i64x2_all_true(i64x2 vec) {
  return wasm_i64x2_all_true(vec);
}
#endif // __wasm_unimplemented_simd128__
i64x2 TESTFN i64x2_shl(i64x2 vec, int32_t shift) {
  return wasm_i64x2_shl(vec, shift);
}

i64x2 TESTFN i64x2_shr_s(i64x2 vec, int32_t shift) {
  return wasm_i64x2_shr(vec, shift);
}
u64x2 TESTFN i64x2_shr_u(u64x2 vec, int32_t shift) {
  return wasm_u64x2_shr(vec, shift);
}
i64x2 TESTFN i64x2_add(i64x2 x, i64x2 y) {
  return wasm_i64x2_add(x, y);
}
i64x2 TESTFN i64x2_sub(i64x2 x, i64x2 y) {
  return wasm_i64x2_sub(x, y);
}
f32x4 TESTFN f32x4_abs(f32x4 vec) {
  return wasm_f32x4_abs(vec);
}
f32x4 TESTFN f32x4_neg(f32x4 vec) {
  return wasm_f32x4_neg(vec);
}
#ifdef __wasm_unimplemented_simd128__
f32x4 TESTFN f32x4_sqrt(f32x4 vec) {
  return wasm_f32x4_sqrt(vec);
}
#endif // __wasm_unimplemented_simd128__
f32x4 TESTFN f32x4_add(f32x4 x, f32x4 y) {
  return wasm_f32x4_add(x,y);
}
f32x4 TESTFN f32x4_sub(f32x4 x, f32x4 y) {
  return wasm_f32x4_sub(x, y);
}
f32x4 TESTFN f32x4_mul(f32x4 x, f32x4 y) {
  return wasm_f32x4_mul(x, y);
}
f32x4 TESTFN f32x4_div(f32x4 x, f32x4 y) {
  return wasm_f32x4_div(x, y);
}
f32x4 TESTFN f32x4_min(f32x4 x, f32x4 y) {
  return wasm_f32x4_min(x, y);
}
f32x4 TESTFN f32x4_max(f32x4 x, f32x4 y) {
  return wasm_f32x4_max(x, y);
}
#ifdef __wasm_unimplemented_simd128__
f64x2 TESTFN f64x2_abs(f64x2 vec) {
  return wasm_f64x2_abs(vec);
}
#endif // __wasm_unimplemented_simd128__
f64x2 TESTFN f64x2_neg(f64x2 vec) {
  return -vec;
}
#ifdef __wasm_unimplemented_simd128__
f64x2 TESTFN f64x2_sqrt(f64x2 vec) {
  return wasm_f64x2_sqrt(vec);
}
#endif // __wasm_unimplemented_simd128__
f64x2 TESTFN f64x2_add(f64x2 x, f64x2 y) {
  return x + y;
}
f64x2 TESTFN f64x2_sub(f64x2 x, f64x2 y) {
  return x - y;
}
f64x2 TESTFN f64x2_mul(f64x2 x, f64x2 y) {
  return x * y;
}
f64x2 TESTFN f64x2_div(f64x2 x, f64x2 y) {
  return x / y;
}
#ifdef __wasm_unimplemented_simd128__
f64x2 TESTFN f64x2_min(f64x2 x, f64x2 y) {
  return wasm_f64x2_min(x, y);
}
f64x2 TESTFN f64x2_max(f64x2 x, f64x2 y) {
  return wasm_f64x2_max(x, y);
}
#endif // __wasm_unimplemented_simd128__
i32x4 TESTFN i32x4_trunc_s_f32x4_sat(f32x4 vec) {
  return __builtin_wasm_trunc_saturate_s_i32x4_f32x4(vec);
}
i32x4 TESTFN i32x4_trunc_u_f32x4_sat(f32x4 vec) {
  return __builtin_wasm_trunc_saturate_u_i32x4_f32x4(vec);
}
#ifdef __wasm_unimplemented_simd128__
i64x2 TESTFN i64x2_trunc_s_f64x2_sat(f64x2 vec) {
  return __builtin_wasm_trunc_saturate_s_i64x2_f64x2(vec);
}
i64x2 TESTFN i64x2_trunc_u_f64x2_sat(f64x2 vec) {
  return __builtin_wasm_trunc_saturate_u_i64x2_f64x2(vec);
}
#endif // __wasm_unimplemented_simd128__
f32x4 TESTFN f32x4_convert_s_i32x4(i32x4 vec) {
  return __builtin_convertvector(vec, f32x4);
}
f32x4 TESTFN f32x4_convert_u_i32x4(i32x4 vec) {
  return __builtin_convertvector((u32x4)vec, f32x4);
}
f64x2 TESTFN f64x2_convert_s_i64x2(i64x2 vec) {
  return __builtin_convertvector(vec, f64x2);
}
f64x2 TESTFN f64x2_convert_u_i64x2(i64x2 vec) {
  return __builtin_convertvector((u64x2)vec, f64x2);
}

static int failures = 0;

#define formatter(x) _Generic((x),                        \
                              int8_t: "%d",               \
                              uint8_t: "%d",              \
                              int16_t: "%d",              \
                              uint16_t: "%d",             \
                              int32_t: "%d",              \
                              uint32_t: "%d",             \
                              int64_t: "%ld",             \
                              uint64_t: "%ld",            \
                              bool: "%d",                 \
                              float: "%f",                \
                              double: "%f"                \
  )

#define err(x) fprintf(stderr, formatter(x), x)

#define eq(a, b) ({                             \
      bool anan = _Generic((a),                 \
                           float: isnan(a),     \
                           double: isnan(a),    \
                           default: false);     \
      bool bnan = _Generic((b),                 \
                           float: isnan(b),     \
                           double: isnan(b),    \
                           default: false);     \
      ((anan && bnan) || (!anan && a == b));    \
    })

#define expect_eq(_a, _b) __extension__({                       \
      __typeof__(_a) a = (_a), b = (_b);                        \
      if (!eq(a, b)) {                                          \
        failures++;                                             \
        fprintf(stderr, "line %d: expected ", __LINE__);        \
        err(b);                                                 \
        fprintf(stderr, ", got ");                              \
        err(a);                                                 \
        fprintf(stderr, "\n");                                  \
      }                                                         \
    })

#define expect_vec(_a, _b) __extension__({                      \
      __typeof__(_a) a = (_a), b = (_b);                        \
      bool err = false;                                         \
      size_t lanes = _Generic((a),                              \
                              u8x16: 16,                        \
                              i8x16: 16,                        \
                              i16x8: 8,                         \
                              u16x8: 8,                         \
                              i32x4: 4,                         \
                              u32x4: 4,                         \
                              i64x2: 2,                         \
                              u64x2: 2,                         \
                              f32x4: 4,                         \
                              f64x2: 2);                        \
      for (size_t i = 0; i < lanes; i++) {                      \
        if (!eq(a[i], b[i])) {                                  \
          err = true;                                           \
          break;                                                \
        }                                                       \
      }                                                         \
      if (err) {                                                \
        failures++;                                             \
        fprintf(stderr, "line %d: expected {", __LINE__);       \
        for (size_t i = 0; i < lanes - 1; i++) {                \
          err(b[i]);                                            \
          fprintf(stderr, ", ");                                \
        }                                                       \
        err(b[lanes - 1]);                                      \
        fprintf(stderr, "}, got {");                            \
        for (size_t i = 0; i < lanes - 1; i++) {                \
          err(a[i]);                                            \
          fprintf(stderr, ", ");                                \
        }                                                       \
        err(a[lanes - 1]);                                      \
        fprintf(stderr, "}\n");                                 \
      }                                                         \
    })

int EMSCRIPTEN_KEEPALIVE main(int argc, char** argv) {
  {
    i8x16 vec = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    expect_vec(i8x16_load(&vec),
              (__extension__(i8x16){3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}));
    i8x16_store(&vec, __extension__(i8x16){7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7});
    expect_vec(i8x16_load(&vec),
              (__extension__(i8x16){7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}));
  }
  expect_vec(i32x4_const(), ((v128){1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}));
  expect_vec(
    i8x16_shuffle_interleave_bytes(
      (i8x16){1, 0, 3, 0, 5, 0, 7, 0, 9, 0, 11, 0, 13, 0, 15, 0},
      (i8x16){0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16}
    ),
    ((i8x16){1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})
  );
  expect_vec(i32x4_shuffle_reverse((i32x4){1, 2, 3, 4}), ((i32x4){4, 3, 2, 1}));

  // i8x16 lane accesses
  expect_vec(i8x16_splat(5), ((i8x16){5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}));
  expect_vec(i8x16_splat(257), ((i8x16){1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}));
  expect_eq(i8x16_extract_lane_s_first((i8x16){-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), -1);
  expect_eq(i8x16_extract_lane_s_last((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1}), -1);
#ifdef __wasm_unimplemented_simd128__
  expect_eq(i8x16_extract_lane_u_first((i8x16){-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), 255);
  expect_eq(i8x16_extract_lane_u_last((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1}), 255);
#endif // __wasm_unimplemented_simd128__
  expect_vec(
    i8x16_replace_lane_first((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 7),
    ((i8x16){7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})
  );
  expect_vec(
    i8x16_replace_lane_last((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 7),
    ((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7})
  );

  // i16x8 lane accesses
  expect_vec(i16x8_splat(5), ((i16x8){5, 5, 5, 5, 5, 5, 5, 5}));
  expect_vec(i16x8_splat(65537), ((i16x8){1, 1, 1, 1, 1, 1, 1, 1}));
  expect_eq(i16x8_extract_lane_s_first((i16x8){-1, 0, 0, 0, 0, 0, 0, 0}), -1);
  expect_eq(i16x8_extract_lane_s_last((i16x8){0, 0, 0, 0, 0, 0, 0, -1}), -1);
#ifdef __wasm_unimplemented_simd128__
  expect_eq(i16x8_extract_lane_u_first((i16x8){-1, 0, 0, 0, 0, 0, 0, 0}), 65535);
  expect_eq(i16x8_extract_lane_u_last((i16x8){0, 0, 0, 0, 0, 0, 0, -1}), 65535);
#endif // __wasm_unimplemented_simd128__
  expect_vec(i16x8_replace_lane_first((i16x8){0, 0, 0, 0, 0, 0, 0, 0}, 7), ((i16x8){7, 0, 0, 0, 0, 0, 0, 0}));
  expect_vec(i16x8_replace_lane_last((i16x8){0, 0, 0, 0, 0, 0, 0, 0}, 7), ((i16x8){0, 0, 0, 0, 0, 0, 0, 7}));

  // i32x4 lane accesses
  expect_vec(i32x4_splat(-5), ((i32x4){-5, -5, -5, -5}));
  expect_eq(i32x4_extract_lane_first((i32x4){-5, 0, 0, 0}), -5);
  expect_eq(i32x4_extract_lane_last((i32x4){0, 0, 0, -5}), -5);
  expect_vec(i32x4_replace_lane_first((i32x4){0, 0, 0, 0}, 53), ((i32x4){53, 0, 0, 0}));
  expect_vec(i32x4_replace_lane_last((i32x4){0, 0, 0, 0}, 53), ((i32x4){0, 0, 0, 53}));

  // i64x2 lane accesses
  expect_vec(i64x2_splat(-5), ((i64x2){-5, -5}));
#ifdef __wasm_unimplemented_simd128__
  expect_eq(i64x2_extract_lane_first((i64x2){-5, 0}), -5);
  expect_eq(i64x2_extract_lane_last((i64x2){0, -5}), -5);
  expect_vec(i64x2_replace_lane_first((i64x2){0, 0}, 53), ((i64x2){53, 0}));
  expect_vec(i64x2_replace_lane_last((i64x2){0, 0}, 53), ((i64x2){0, 53}));
#endif // __wasm_unimplemented_simd128__

  // f32x4 lane accesses
  expect_vec(f32x4_splat(-5), ((f32x4){-5, -5, -5, -5}));
  expect_eq(f32x4_extract_lane_first((f32x4){-5, 0, 0, 0}), -5);
  expect_eq(f32x4_extract_lane_last((f32x4){0, 0, 0, -5}), -5);
  expect_vec(f32x4_replace_lane_first((f32x4){0, 0, 0, 0}, 53), ((f32x4){53, 0, 0, 0}));
  expect_vec(f32x4_replace_lane_last((f32x4){0, 0, 0, 0}, 53), ((f32x4){0, 0, 0, 53}));

#ifdef __wasm_unimplemented_simd128__
  // f64x2 lane accesses
  expect_vec(f64x2_splat(-5), ((f64x2){-5, -5}));
  expect_eq(f64x2_extract_lane_first((f64x2){-5, 0}), -5);
  expect_eq(f64x2_extract_lane_last((f64x2){0, -5}), -5);
  expect_vec(f64x2_replace_lane_first((f64x2){0, 0}, 53), ((f64x2){53, 0}));
  expect_vec(f64x2_replace_lane_last((f64x2){0, 0}, 53), ((f64x2){0, 53}));
#endif // __wasm_unimplemented_simd128__

  // i8x16 comparisons
  expect_vec(
    i8x16_eq(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){-1, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0})
  );
  expect_vec(
    i8x16_ne(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){0, -1, 0, -1, -1, -1, -1, -1, 0, -1, -1, 0, -1, -1, -1, -1})
  );
  expect_vec(
    i8x16_lt_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){0, 0, 0, -1, 0, -1, -1, 0, 0, 0, -1, 0, 0, -1, -1, 0})
  );
  expect_vec(
    i8x16_lt_u(
      (u8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (u8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){0, -1, 0, 0, -1, -1, 0, -1, 0, -1, 0, 0, -1, -1, 0, -1})
  );
  expect_vec(
    i8x16_gt_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1})
  );
  expect_vec(
    i8x16_gt_u(
      (u8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (u8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){0, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1, 0})
  );
  expect_vec(
    i8x16_le_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){-1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0})
  );
  // bugs.chromium.org/p/v8/issues/detail?id=8635
  // expect_vec(
  //   i8x16_le_u(
  //     (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
  //     (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
  //   ),
  //   ((i8x16){-1, -1, -1, 0, -1, -1, 0, -1, -1, -1, 0, -1, -1, -1, 0, -1})
  // );
  expect_vec(
    i8x16_ge_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((u8x16){-1, -1, -1, 0, -1, 0, 0, -1, -1, -1, 0, -1, -1, 0, 0, -1})
  );
  // expect_vec(
  //   i8x16_ge_u(
  //     (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
  //     (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
  //   ),
  //   ((i8x16){-1, 0, -1, -1, 0, 0, -1, 0, -1, 0, -1, -1, 0, 0, -1, 0})
  // );

  // i16x8 comparisons
  expect_vec(
    i16x8_eq(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){-1, 0, 0, 0, 0, 0, 0, 0})
  );
  expect_vec(
    i16x8_ne(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){0, -1, -1, -1, -1, -1, -1, -1})
  );
  expect_vec(
    i16x8_lt_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){0, 0, 0, -1, 0, -1, 0, -1})
  );
  expect_vec(
    i16x8_lt_u(
      (u16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (u16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){0, 0, 0, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_gt_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){0, -1, -1, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_gt_u(
      (u16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (u16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){0, -1, -1, -1, 0, -1, 0, -1})
  );
  expect_vec(
    i16x8_le_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){-1, 0, 0, -1, 0, -1, 0, -1})
  );
  expect_vec(
    i16x8_le_u(
      (u16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (u16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){-1, 0, 0, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_ge_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){-1, -1, -1, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_ge_u(
      (u16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (u16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((u16x8){-1, -1, -1, -1, 0, -1, 0, -1})
  );

  // i342x4 comparisons
  expect_vec(
    i32x4_eq((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((u32x4){-1, 0, 0, 0})
  );
  expect_vec(
    i32x4_ne((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((u32x4){0, -1, -1, -1})
  );
  expect_vec(
    i32x4_lt_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((u32x4){0, -1, 0, -1})
  );
  expect_vec(
    i32x4_lt_u((u32x4){0, -1, 53, -7}, (u32x4){0, 53, -7, -1}), ((u32x4){0, 0, -1, -1})
  );
  expect_vec(
    i32x4_gt_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((u32x4){0, 0, -1, 0})
  );
  expect_vec(
    i32x4_gt_u((u32x4){0, -1, 53, -7}, (u32x4){0, 53, -7, -1}), ((u32x4){0, -1, 0, 0})
  );
  expect_vec(
    i32x4_le_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((u32x4){-1, -1, 0, -1})
  );
  expect_vec(
    i32x4_le_u((u32x4){0, -1, 53, -7}, (u32x4){0, 53, -7, -1}), ((u32x4){-1, 0, -1, -1})
  );
  expect_vec(
    i32x4_ge_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((u32x4){-1, 0, -1, 0})
  );
  expect_vec(
    i32x4_ge_u((u32x4){0, -1, 53, -7}, (u32x4){0, 53, -7, -1}), ((u32x4){-1, -1, 0, 0})
  );

  // f32x4 comparisons
  expect_vec(
    f32x4_eq((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((u32x4){-1, 0, 0, 0})
  );
  expect_vec(
    f32x4_ne((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((u32x4){0, -1, -1, -1})
  );
  expect_vec(
    f32x4_lt((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((u32x4){0, -1, 0, -1})
  );
  expect_vec(
    f32x4_gt((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((u32x4){0, 0, -1, 0})
  );
  expect_vec(
    f32x4_le((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((u32x4){-1, -1, 0, -1})
  );
  expect_vec(
    f32x4_ge((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((u32x4){-1, 0, -1, 0})
  );
  expect_vec(
    f32x4_eq((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((u32x4){0, 0, 0, -1})
  );
  expect_vec(
    f32x4_ne((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((u32x4){-1, -1, -1, 0})
  );
  expect_vec(
    f32x4_lt((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((u32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_gt((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((u32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_le((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((u32x4){0, 0, 0, -1})
  );
  expect_vec(
    f32x4_ge((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((u32x4){0, 0, 0, -1})
  );
  expect_vec(
    f32x4_eq((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((u32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_ne((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((u32x4){-1, -1, -1, -1})
  );
  expect_vec(
    f32x4_lt((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((u32x4){-1, -1, 0, 0})
  );
  expect_vec(
    f32x4_gt((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((u32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_le((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((u32x4){-1, -1, 0, 0})
  );
  expect_vec(
    f32x4_ge((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((u32x4){0, 0, 0, 0})
  );

#ifdef __wasm_undefined_simd128__
  // f64x2 comparisons
  expect_vec(f64x2_eq((f64x2){0, 1}, (f64x2){0, 0}), ((u64x2){-1, 0}));
  expect_vec(f64x2_ne((f64x2){0, 1}, (f64x2){0, 0}), ((u64x2){0, -1}));
  expect_vec(f64x2_lt((f64x2){0, 1}, (f64x2){0, 0}), ((u64x2){0, 0}));
  expect_vec(f64x2_gt((f64x2){0, 1}, (f64x2){0, 0}), ((u64x2){0, -1}));
  expect_vec(f64x2_le((f64x2){0, 1}, (f64x2){0, 0}), ((u64x2){-1, 0}));
  expect_vec(f64x2_ge((f64x2){0, 1}, (f64x2){0, 0}), ((u64x2){-1, -1}));
  expect_vec(f64x2_eq((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((u64x2){0, 0}));
  expect_vec(f64x2_ne((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((u64x2){-1, -1}));
  expect_vec(f64x2_lt((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((u64x2){0, -1}));
  expect_vec(f64x2_gt((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((u64x2){0, 0}));
  expect_vec(f64x2_le((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((u64x2){0, -1}));
  expect_vec(f64x2_ge((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((u64x2){0, 0}));
#endif // __wasm_undefined_simd128__

  // bitwise operations
  expect_vec(v128_not((v128)(i32x4){0, -1, 0, -1}), (v128)((i32x4){-1, 0, -1, 0}));
  expect_vec(
    v128_and((v128)(i32x4){0, 0, -1, -1}, (v128)(i32x4){0, -1, 0, -1}),
    (v128)((i32x4){0, 0, 0, -1})
  );
  expect_vec(
    v128_or((v128)(i32x4){0, 0, -1, -1}, (v128)(i32x4){0, -1, 0, -1}),
    (v128)((i32x4){0, -1, -1, -1})
  );
  expect_vec(
    v128_xor((v128)(i32x4){0, 0, -1, -1}, (v128)(i32x4){0, -1, 0, -1}),
    (v128)((i32x4){0, -1, -1, 0})
  );
  expect_vec(
    v128_bitselect(
      (v128)(i32x4){0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA},
      (v128)(i32x4){0xBBBBBBBB, 0xBBBBBBBB, 0xBBBBBBBB, 0xBBBBBBBB},
      (v128)(i32x4){0xF0F0F0F0, 0xFFFFFFFF, 0x00000000, 0xFF00FF00}
    ),
    (v128)((i32x4){0xABABABAB, 0xAAAAAAAA, 0xBBBBBBBB, 0xAABBAABB})
  );

  // i8x16 arithmetic
  expect_vec(
    i8x16_neg((i8x16){0, 1, 42, -3, -56, 127, -128, -126, 0, -1, -42, 3, 56, -127, -128, 126}),
    ((i8x16){0, -1, -42, 3, 56, -127, -128, 126, 0, 1, 42, -3, -56, 127, -128, -126})
  );
  expect_eq(i8x16_any_true((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), 0);
  expect_eq(i8x16_any_true((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}), 1);
  expect_eq(i8x16_any_true((i8x16){1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}), 1);
  expect_eq(i8x16_any_true((i8x16){1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}), 1);
  expect_eq(i8x16_all_true((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), 0);
  expect_eq(i8x16_all_true((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}), 0);
  expect_eq(i8x16_all_true((i8x16){1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}), 0);
  expect_eq(i8x16_all_true((i8x16){1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}), 1);
  expect_vec(
    i8x16_shl((i8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64}, 1),
    ((i8x16){0, 2, 4, 8, 16, 32, 64, -128, 0, 6, 12, 24, 48, 96, -64, -128})
  );
  expect_vec(
    i8x16_shl((i8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64}, 8),
    ((i8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64})
  );
  expect_vec(
    i8x16_shr_u((u8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64}, 1),
    ((u8x16){0, 0, 1, 2, 4, 8, 16, 32, 64, 1, 3, 6, 12, 24, 48, 96})
  );
  expect_vec(
    i8x16_shr_u((u8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64}, 8),
    ((u8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64})
  );
  expect_vec(
    i8x16_shr_s((i8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64}, 1),
    ((i8x16){0, 0, 1, 2, 4, 8, 16, 32, -64, 1, 3, 6, 12, 24, 48, -32})
  );
  expect_vec(
    i8x16_shr_s((i8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64}, 8),
    ((i8x16){0, 1, 2, 4, 8, 16, 32, 64, -128, 3, 6, 12, 24, 48, 96, -64})
  );
  expect_vec(
    i8x16_add(
      (i8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (i8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((i8x16){3, 17, 0, 0, 0, 135, 109, 46, 145, 225, 48, 184, 17, 249, 128, 215})
  );
  expect_vec(
    i8x16_add_saturate_s(
      (i8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (i8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((i8x16){3, 17, 0, 128, 0, 135, 109, 46, 127, 225, 48, 184, 17, 249, 127, 215})
  );
  expect_vec(
    i8x16_add_saturate_u(
      (u8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (u8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((u8x16){3, 255, 255, 255, 255, 135, 109, 46, 145, 225, 255, 184, 17, 255, 128, 215})
  );
  expect_vec(
    i8x16_sub(
      (i8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (i8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((i8x16){253, 67, 254, 0, 254, 123, 159, 12, 61, 167, 158, 100, 17, 251, 130, 187})
  );
  expect_vec(
    i8x16_sub_saturate_s(
      (i8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (i8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((i8x16){253, 67, 254, 0, 127, 128, 159, 12, 61, 167, 158, 128, 17, 251, 130, 127})
  );
  expect_vec(
    i8x16_sub_saturate_u(
      (u8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (u8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((u8x16){0, 0, 254, 0, 0, 123, 0, 12, 61, 167, 158, 100, 17, 0, 0, 0})
  );
  expect_vec(
    i8x16_mul(
      (i8x16){0, 42, 255, 128, 127, 129, 6, 29, 103, 196, 231, 142, 17, 250, 1, 73},
      (i8x16){3, 231, 1, 128, 129, 6, 103, 17, 42, 29, 73, 42, 0, 255, 127, 142}
    ),
    ((i8x16){0, 230, 255, 0, 255, 6, 106, 237, 230, 52, 223, 76, 0, 6, 127, 126})
  );

  // i16x8 arithmetic
  expect_vec(
    i16x8_neg((i16x8){0, 1, 42, -3, -56, 32767, -32768, 32766}),
    ((i16x8){0, -1, -42, 3, 56, -32767, -32768, -32766})
  );
  expect_eq(i16x8_any_true((i16x8){0, 0, 0, 0, 0, 0, 0, 0}), 0);
  expect_eq(i16x8_any_true((i16x8){0, 0, 1, 0, 0, 0, 0, 0}), 1);
  expect_eq(i16x8_any_true((i16x8){1, 1, 1, 1, 1, 0, 1, 1}), 1);
  expect_eq(i16x8_any_true((i16x8){1, 1, 1, 1, 1, 1, 1, 1}), 1);
  expect_eq(i16x8_all_true((i16x8){0, 0, 0, 0, 0, 0, 0, 0}), 0);
  expect_eq(i16x8_all_true((i16x8){0, 0, 1, 0, 0, 0, 0, 0}), 0);
  expect_eq(i16x8_all_true((i16x8){1, 1, 1, 1, 1, 0, 1, 1}), 0);
  expect_eq(i16x8_all_true((i16x8){1, 1, 1, 1, 1, 1, 1, 1}), 1);
  expect_vec(
    i16x8_shl((i16x8){0, 8, 16, 128, 256, 2048, 4096, -32768}, 1),
    ((i16x8){0, 16, 32, 256, 512, 4096, 8192, 0})
  );
  expect_vec(
    i16x8_shl((i16x8){0, 8, 16, 128, 256, 2048, 4096, -32768}, 16),
    ((i16x8){0, 8, 16, 128, 256, 2048, 4096, -32768})
  );
  expect_vec(
    i16x8_shr_u((u16x8){0, 8, 16, 128, 256, 2048, 4096, -32768}, 1),
    ((u16x8){0, 4, 8, 64, 128, 1024, 2048, 16384})
  );
  expect_vec(
    i16x8_shr_u((u16x8){0, 8, 16, 128, 256, 2048, 4096, -32768}, 16),
    ((u16x8){0, 8, 16, 128, 256, 2048, 4096, -32768})
  );
  expect_vec(
    i16x8_shr_s((i16x8){0, 8, 16, 128, 256, 2048, 4096, -32768}, 1),
    ((i16x8){0, 4, 8, 64, 128, 1024, 2048, -16384})
  );
  expect_vec(
    i16x8_shr_s((i16x8){0, 8, 16, 128, 256, 2048, 4096, -32768}, 16),
    ((i16x8){0, 8, 16, 128, 256, 2048, 4096, -32768})
  );
  expect_vec(
    i16x8_add(
      (i16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (i16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((i16x8){768, -255, 0, 0, -30976, 12288, -1792, -32768})
  );
  expect_vec(
    i16x8_add_saturate_s(
      (i16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (i16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((i16x8){768, -255, -32768, 0, -30976, 12288, -1792, 32767})
  );
  expect_vec(
    i16x8_add_saturate_u(
      (u16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (u16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((u16x8){768, -255, -1, -1, -30976, -1, -1, -32768})
  );
  expect_vec(
    i16x8_sub(
      (i16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (i16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((i16x8){-768, -257, 0, -512, 31488, -25088, -1280, 32764})
  );
  expect_vec(
    i16x8_sub_saturate_s(
      (i16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (i16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((i16x8){-768, -257, 0, 32767, -32768, -25088, -1280, 32764})
  );
  expect_vec(
    i16x8_sub_saturate_u(
      (u16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (u16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((u16x8){0, -257, 0, 0, 31488, -25088, 0, 32764})
  );
  expect_vec(
    i16x8_mul(
      (i16x8){0, -256, -32768, 32512, -32512, -6400, -1536, 32766},
      (i16x8){768, 1, -32768, -32512, 1536, 18688, -256, 2}
    ),
    ((i16x8){0, -256, 0, 0, 0, 0, 0, -4})
  );

  // i32x4 arithmetic
  expect_vec(i32x4_neg((i32x4){0, 1, 0x80000000, 0x7fffffff}), ((i32x4){0, -1, 0x80000000, 0x80000001}));
  expect_eq(i32x4_any_true((i32x4){0, 0, 0, 0}), 0);
  expect_eq(i32x4_any_true((i32x4){0, 0, 1, 0}), 1);
  expect_eq(i32x4_any_true((i32x4){1, 0, 1, 1}), 1);
  expect_eq(i32x4_any_true((i32x4){1, 1, 1, 1}), 1);
  expect_eq(i32x4_all_true((i32x4){0, 0, 0, 0}), 0);
  expect_eq(i32x4_all_true((i32x4){0, 0, 1, 0}), 0);
  expect_eq(i32x4_all_true((i32x4){1, 0, 1, 1}), 0);
  expect_eq(i32x4_all_true((i32x4){1, 1, 1, 1}), 1);
  expect_vec(i32x4_shl((i32x4){1, 0x40000000, 0x80000000, -1}, 1), ((i32x4){2, 0x80000000, 0, -2}));
  expect_vec(i32x4_shl((i32x4){1, 0x40000000, 0x80000000, -1}, 32), ((i32x4){1, 0x40000000, 0x80000000, -1}));
  expect_vec(i32x4_shr_s((i32x4){1, 0x40000000, 0x80000000, -1}, 1), ((i32x4){0, 0x20000000, 0xc0000000, -1}));
  expect_vec(i32x4_shr_s((i32x4){1, 0x40000000, 0x80000000, -1}, 32), ((i32x4){1, 0x40000000, 0x80000000, -1}));
  expect_vec(i32x4_shr_u((u32x4){1, 0x40000000, 0x80000000, -1}, 1), ((u32x4){0, 0x20000000, 0x40000000, 0x7fffffff}));
  expect_vec(i32x4_shr_u((u32x4){1, 0x40000000, 0x80000000, -1}, 32), ((u32x4){1, 0x40000000, 0x80000000, -1}));
  expect_vec(i32x4_add((i32x4){0, 0x80000001, 42, 5}, (i32x4){0, 0x80000001, 5, 42}), ((i32x4){0, 2, 47, 47}));
  expect_vec(i32x4_sub((i32x4){0, 2, 47, 47}, (i32x4){0, 0x80000001, 42, 5}), ((i32x4){0, 0x80000001, 5, 42}));
  expect_vec(i32x4_mul((i32x4){0, 0x80000001, 42, 5}, (i32x4){0, 0x80000001, 42, 5}), ((i32x4){0, 1, 1764, 25}));


  // i64x2 arithmetic
  expect_vec(i64x2_neg((i64x2){0x8000000000000000, 42}), ((i64x2){0x8000000000000000, -42}));
#ifdef __wasm_unimplemented_simd128__
  expect_eq(i64x2_any_true((i64x2){0, 0}), 0);
  expect_eq(i64x2_any_true((i64x2){1, 0}), 1);
  expect_eq(i64x2_any_true((i64x2){1, 1}), 1);
  expect_eq(i64x2_all_true((i64x2){0, 0}), 0);
  expect_eq(i64x2_all_true((i64x2){1, 0}), 0);
  expect_eq(i64x2_all_true((i64x2){1, 1}), 1);
#endif // __wasm_unimplemented_simd128__
  expect_vec(i64x2_shl((i64x2){1, 0x8000000000000000}, 1), ((i64x2){2, 0}));
  expect_vec(i64x2_shl((i64x2){1, 0x8000000000000000}, 64), ((i64x2){1, 0x8000000000000000}));
  expect_vec(i64x2_shr_s((i64x2){1, 0x8000000000000000}, 1), ((i64x2){0, 0xc000000000000000}));
  expect_vec(i64x2_shr_s((i64x2){1, 0x8000000000000000}, 64), ((i64x2){1, 0x8000000000000000}));
  expect_vec(i64x2_shr_u((u64x2){1, 0x8000000000000000}, 1), ((u64x2){0, 0x4000000000000000}));
  expect_vec(i64x2_shr_u((u64x2){1, 0x8000000000000000}, 64), ((u64x2){1, 0x8000000000000000}));
  expect_vec(i64x2_add((i64x2){0x8000000000000001, 42}, (i64x2){0x8000000000000001, 0}), ((i64x2){2, 42}));
  expect_vec(i64x2_sub((i64x2){2, 42}, (i64x2){0x8000000000000001, 0}), ((i64x2){0x8000000000000001, 42}));

  // f32x4 arithmetic
  expect_vec(f32x4_abs((f32x4){-0., NAN, -INFINITY, 5}), ((f32x4){0, NAN, INFINITY, 5}));
  expect_vec(f32x4_neg((f32x4){-0., NAN, -INFINITY, 5}), ((f32x4){0, -NAN, INFINITY, -5}));
#ifdef __wasm_unimplemented_simd128__
  expect_vec(f32x4_sqrt((f32x4){-0., NAN, INFINITY, 4}), ((f32x4){-0., NAN, INFINITY, 2}));
#endif // __wasm_unimplemented_simd128__
  expect_vec(f32x4_add((f32x4){NAN, -NAN, INFINITY, 42}, (f32x4){42, INFINITY, INFINITY, 1}), ((f32x4){NAN, -NAN, INFINITY, 43}));
  expect_vec(f32x4_sub((f32x4){NAN, -NAN, INFINITY, 42}, (f32x4){42, INFINITY, -INFINITY, 1}), ((f32x4){NAN, -NAN, INFINITY, 41}));
  expect_vec(f32x4_mul((f32x4){NAN, -NAN, INFINITY, 42}, (f32x4){42, INFINITY, INFINITY, 2}), ((f32x4){NAN, -NAN, INFINITY, 84}));
  expect_vec(f32x4_div((f32x4){NAN, -NAN, INFINITY, 42}, (f32x4){42, INFINITY, 2, 2}), ((f32x4){NAN, -NAN, INFINITY, 21}));
  // expect_vec(f32x4_min((f32x4){-0., 0, NAN, 5}, (f32x4){0, -0., 5, NAN}), ((f32x4){-0., -0., NAN, NAN}));
  // expect_vec(f32x4_max((f32x4){-0., 0, NAN, 5}, (f32x4){0, -0., 5, NAN}), ((f32x4){0, 0, NAN, NAN}));

  // f64x2 arithmetic
#ifdef __wasm_unimplemented_simd128__
  expect_vec(f64x2_abs((f64x2){-0., NAN}), ((f64x2){0, NAN}));
  expect_vec(f64x2_abs((f64x2){-INFINITY, 5}), ((f64x2){INFINITY, 5}));
#endif // __wasm_unimplemented_simd128__
  expect_vec(f64x2_neg((f64x2){-0., NAN}), ((f64x2){0, -NAN}));
  expect_vec(f64x2_neg((f64x2){-INFINITY, 5}), ((f64x2){INFINITY, -5}));
#ifdef __wasm_unimplemented_simd128__
  expect_vec(f64x2_sqrt((f64x2){-0., NAN}), ((f64x2){-0., NAN}));
  expect_vec(f64x2_sqrt((f64x2){INFINITY, 4}), ((f64x2){INFINITY, 2}));
#endif // __wasm_unimplemented_simd128__
  expect_vec(f64x2_add((f64x2){NAN, -NAN}, (f64x2){42, INFINITY}), ((f64x2){NAN, -NAN}));
  expect_vec(f64x2_add((f64x2){INFINITY, 42}, (f64x2){INFINITY, 1}), ((f64x2){INFINITY, 43}));
  expect_vec(f64x2_sub((f64x2){NAN, -NAN}, (f64x2){42, INFINITY}), ((f64x2){NAN, -NAN}));
  expect_vec(f64x2_sub((f64x2){INFINITY, 42}, (f64x2){-INFINITY, 1}), ((f64x2){INFINITY, 41}));
  expect_vec(f64x2_mul((f64x2){NAN, -NAN}, (f64x2){42, INFINITY}), ((f64x2){NAN, -NAN}));
  expect_vec(f64x2_mul((f64x2){INFINITY, 42}, (f64x2){INFINITY, 2}), ((f64x2){INFINITY, 84}));
  expect_vec(f64x2_div((f64x2){NAN, -NAN}, (f64x2){42, INFINITY}), ((f64x2){NAN, -NAN}));
  expect_vec(f64x2_div((f64x2){INFINITY, 42}, (f64x2){2, 2}), ((f64x2){INFINITY, 21}));
#ifdef __wasm_unimplemented_simd128__
  expect_vec(f64x2_min((f64x2){-0., 0}, (f64x2){0, -0}), ((f64x2){-0., -0}));
  expect_vec(f64x2_min((f64x2){NAN, 5}, (f64x2){5, NAN}), ((f64x2){NAN, NAN}));
  expect_vec(f64x2_max((f64x2){-0., 0}, (f64x2){0, -0}), ((f64x2){0, 0}));
  expect_vec(f64x2_max((f64x2){NAN, 5}, (f64x2){5, NAN}), ((f64x2){NAN, NAN}));
#endif // __wasm_unimplemented_simd128__

  // conversions
  expect_vec(i32x4_trunc_s_f32x4_sat((f32x4){42, NAN, INFINITY, -INFINITY}), ((i32x4){42, 0, 2147483647, -2147483648ll}));
  expect_vec(i32x4_trunc_u_f32x4_sat((f32x4){42, NAN, INFINITY, -INFINITY}), ((i32x4){42, 0, 4294967295ull, 0}));
#ifdef __wasm_unimplemented_simd128__
  expect_vec(i64x2_trunc_s_f64x2_sat((f64x2){42, NAN}), ((i64x2){42, 0}));
  expect_vec(i64x2_trunc_s_f64x2_sat((f64x2){INFINITY, -INFINITY}), ((i64x2){9223372036854775807ll, -9223372036854775807ll - 1}));
  expect_vec(i64x2_trunc_u_f64x2_sat((f64x2){42, NAN}), ((i64x2){42, 0}));
  expect_vec(i64x2_trunc_u_f64x2_sat((f64x2){INFINITY, -INFINITY}), ((i64x2){18446744073709551615ull, 0}));
#endif // __wasm_unimplemented_simd128__
  expect_vec(f32x4_convert_s_i32x4((i32x4){0, -1, 2147483647, -2147483647 - 1}), ((f32x4){0, -1, 2147483648., -2147483648.}));
  expect_vec(f32x4_convert_u_i32x4((i32x4){0, -1, 2147483647, -2147483647 - 1}), ((f32x4){0, 4294967296., 2147483648., 2147483648.}));
  expect_vec(f64x2_convert_s_i64x2((i64x2){0, -1}), ((f64x2){0, -1}));
  expect_vec(f64x2_convert_s_i64x2((i64x2){9223372036854775807, -9223372036854775807 - 1}), ((f64x2){9223372036854775807., -9223372036854775808.}));
  expect_vec(f64x2_convert_u_i64x2((i64x2){0, -1}), ((f64x2){0, 18446744073709551616.}));
  expect_vec(f64x2_convert_u_i64x2((i64x2){9223372036854775807 , -9223372036854775808.}), ((f64x2){9223372036854775807., 9223372036854775808.}));

  if (failures == 0) {
    printf("Success!\n");
  } else {
    printf("Failed :(\n");
  }
}
