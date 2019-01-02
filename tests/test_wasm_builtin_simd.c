#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <emscripten.h>

typedef char i8x16 __attribute((vector_size(16)));
typedef short i16x8 __attribute((vector_size(16)));
typedef int i32x4 __attribute((vector_size(16)));
typedef long long i64x2 __attribute((vector_size(16)));
typedef unsigned char u8x16 __attribute((vector_size(16)));
typedef unsigned short u16x8 __attribute((vector_size(16)));
typedef unsigned int u32x4 __attribute((vector_size(16)));
typedef unsigned long long u64x2 __attribute((vector_size(16)));
typedef float f32x4 __attribute((vector_size(16)));
typedef double f64x2 __attribute((vector_size(16)));
typedef u8x16 v128;

#define TESTFN EMSCRIPTEN_KEEPALIVE __attribute__((noinline))

v128 TESTFN v128_load(v128 *ptr) {
  return *ptr;
}
void TESTFN v128_store(v128 *ptr, v128 vec) {
  *ptr = vec;
}
i32x4 TESTFN i32x4_const(void) {
  return (i32x4) {1, 2, 3, 4};
}
// v128 TESTFN v128_shuffle_interleave_bytes(v128 x, v128 y) {
//   return __builtin_shufflevector(x, y, 0, 17, 2, 19, 4, 21, 6, 23, 8, 25, 10, 27, 12, 29, 14, 31);
// }
// i32x4 TESTFN v128_shuffle_reverse_i32s(i32x4 vec) {
//   return __builtin_shufflevector(vec, vec, 3, 2, 1, 0);
// }
i8x16 TESTFN i8x16_splat(int32_t x) {
  return (i8x16) {x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x};
}
int32_t TESTFN i8x16_extract_lane_s_first(i8x16 vec) {
  return __builtin_wasm_extract_lane_s_i8x16(vec, 0);
}
int32_t TESTFN i8x16_extract_lane_s_last(i8x16 vec) {
  return __builtin_wasm_extract_lane_s_i8x16(vec, 15);
}
uint32_t TESTFN i8x16_extract_lane_u_first(i8x16 vec) {
  return __builtin_wasm_extract_lane_u_i8x16(vec, 0);
}
uint32_t TESTFN i8x16_extract_lane_u_last(i8x16 vec) {
  return __builtin_wasm_extract_lane_u_i8x16(vec, 15);
}
i8x16 TESTFN i8x16_replace_lane_first(i8x16 vec, int32_t val) {
  return __builtin_wasm_replace_lane_i8x16(vec, 0, val);
}
i8x16 TESTFN i8x16_replace_lane_last(i8x16 vec, int32_t val) {
  return __builtin_wasm_replace_lane_i8x16(vec, 15, val);
}
i16x8 TESTFN i16x8_splat(int32_t x) {
  return (i16x8) {x, x, x, x, x, x, x, x};
}
int32_t TESTFN i16x8_extract_lane_s_first(i16x8 vec) {
  return __builtin_wasm_extract_lane_s_i16x8(vec, 0);
}
int32_t TESTFN i16x8_extract_lane_s_last(i16x8 vec) {
  return __builtin_wasm_extract_lane_s_i16x8(vec, 7);
}
int32_t TESTFN i16x8_extract_lane_u_first(i16x8 vec) {
  return __builtin_wasm_extract_lane_u_i16x8(vec, 0);
}
int32_t TESTFN i16x8_extract_lane_u_last(i16x8 vec) {
  return __builtin_wasm_extract_lane_u_i16x8(vec, 7);
}
i16x8 TESTFN i16x8_replace_lane_first(i16x8 vec, int32_t val) {
  return __builtin_wasm_replace_lane_i16x8(vec, 0, val);
}
i16x8 TESTFN i16x8_replace_lane_last(i16x8 vec, int32_t val) {
  return __builtin_wasm_replace_lane_i16x8(vec, 7, val);
}
i32x4 TESTFN i32x4_splat(int32_t x) {
  return (i32x4) {x, x, x, x};
}
int32_t TESTFN i32x4_extract_lane_first(i32x4 vec) {
  return __builtin_wasm_extract_lane_i32x4(vec, 0);
}
int32_t TESTFN i32x4_extract_lane_last(i32x4 vec) {
  return __builtin_wasm_extract_lane_i32x4(vec, 3);
}
i32x4 TESTFN i32x4_replace_lane_first(i32x4 vec, int32_t val) {
  return __builtin_wasm_replace_lane_i32x4(vec, 0, val);
}
i32x4 TESTFN i32x4_replace_lane_last(i32x4 vec, int32_t val) {
  return __builtin_wasm_replace_lane_i32x4(vec, 3, val);
}
i64x2 TESTFN i64x2_splat(int64_t x) {
  return (i64x2) {x, x};
}
int64_t TESTFN i64x2_extract_lane_first(i64x2 vec) {
  return __builtin_wasm_extract_lane_i64x2(vec, 0);
}
int64_t TESTFN i64x2_extract_lane_last(i64x2 vec) {
  return __builtin_wasm_extract_lane_i64x2(vec, 1);
}
i64x2 TESTFN i64x2_replace_lane_first(i64x2 vec, int64_t val) {
  return __builtin_wasm_replace_lane_i64x2(vec, 0, val);
}
i64x2 TESTFN i64x2_replace_lane_last(i64x2 vec, int64_t val) {
  return __builtin_wasm_replace_lane_i64x2(vec, 1, val);
}
f32x4 TESTFN f32x4_splat(float x) {
  return (f32x4) {x, x, x, x};
}
float TESTFN f32x4_extract_lane_first(f32x4 vec) {
  return __builtin_wasm_extract_lane_f32x4(vec, 0);
}
float TESTFN f32x4_extract_lane_last(f32x4 vec) {
  return __builtin_wasm_extract_lane_f32x4(vec, 3);
}
f32x4 TESTFN f32x4_replace_lane_first(f32x4 vec, float val) {
  return __builtin_wasm_replace_lane_f32x4(vec, 0, val);
}
f32x4 TESTFN f32x4_replace_lane_last(f32x4 vec, float val) {
  return __builtin_wasm_replace_lane_f32x4(vec, 3, val);
}
f64x2 TESTFN f64x2_splat(int64_t x) {
  return (f64x2) {x, x};
}
double TESTFN f64x2_extract_lane_first(f64x2 vec) {
  return __builtin_wasm_extract_lane_f64x2(vec, 0);
}
double TESTFN f64x2_extract_lane_last(f64x2 vec) {
  return __builtin_wasm_extract_lane_f64x2(vec, 1);
}
f64x2 TESTFN f64x2_replace_lane_first(f64x2 vec, double val) {
  return __builtin_wasm_replace_lane_f64x2(vec, 0, val);
}
f64x2 TESTFN f64x2_replace_lane_last(f64x2 vec, double val) {
  return __builtin_wasm_replace_lane_f64x2(vec, 1, val);
}
i8x16 TESTFN i8x16_eq(i8x16 x, i8x16 y) {
  return x == y;
}
i8x16 TESTFN i8x16_ne(i8x16 x, i8x16 y) {
  return x != y;
}
i8x16 TESTFN i8x16_lt_s(i8x16 x, i8x16 y) {
  return x < y;
}
i8x16 TESTFN i8x16_lt_u(i8x16 x, i8x16 y) {
  return (u8x16)x < (u8x16)y;
}
i8x16 TESTFN i8x16_gt_s(i8x16 x, i8x16 y) {
  return x > y;
}
i8x16 TESTFN i8x16_gt_u(i8x16 x, i8x16 y) {
  return (u8x16)x > (u8x16)y;
}
i8x16 TESTFN i8x16_le_s(i8x16 x, i8x16 y) {
  return x <= y;
}
i8x16 TESTFN i8x16_le_u(i8x16 x, i8x16 y) {
  return (u8x16)x <= (u8x16)y;
}
i8x16 TESTFN i8x16_ge_s(i8x16 x, i8x16 y) {
  return x >= y;
}
i8x16  TESTFN i8x16_ge_u(i8x16 x, i8x16 y) {
  return (u8x16)x >= (u8x16)y;
}
i16x8 TESTFN i16x8_eq(i16x8 x, i16x8 y) {
  return x == y;
}
i16x8 TESTFN i16x8_ne(i16x8 x, i16x8 y) {
  return x != y;
}
i16x8 TESTFN i16x8_lt_s(i16x8 x, i16x8 y) {
  return x < y;
}
i16x8 TESTFN i16x8_lt_u(i16x8 x, i16x8 y) {
  return (u16x8)x < (u16x8)y;
}
i16x8 TESTFN i16x8_gt_s(i16x8 x, i16x8 y) {
  return x > y;
}
i16x8 TESTFN i16x8_gt_u(i16x8 x, i16x8 y) {
  return (u16x8)x > (u16x8)y;
}
i16x8 TESTFN i16x8_le_s(i16x8 x, i16x8 y) {
  return x <= y;
}
i16x8 TESTFN i16x8_le_u(i16x8 x, i16x8 y) {
  return (u16x8)x <= (u16x8)y;
}
i16x8 TESTFN i16x8_ge_s(i16x8 x, i16x8 y) {
  return x >= y;
}
i16x8  TESTFN i16x8_ge_u(i16x8 x, i16x8 y) {
  return (u16x8)x >= (u16x8)y;
}
i32x4 TESTFN i32x4_eq(i32x4 x, i32x4 y) {
  return (i32x4)(x == y);
}
i32x4 TESTFN i32x4_ne(i32x4 x, i32x4 y) {
  return (i32x4)(x != y);
}
i32x4 TESTFN i32x4_lt_s(i32x4 x, i32x4 y) {
  return (i32x4)(x < y);
}
i32x4 TESTFN i32x4_lt_u(i32x4 x, i32x4 y) {
  return (i32x4)((u32x4)x < (u32x4)y);
}
i32x4 TESTFN i32x4_gt_s(i32x4 x, i32x4 y) {
  return (i32x4)(x > y);
}
i32x4 TESTFN i32x4_gt_u(i32x4 x, i32x4 y) {
  return (i32x4)((u32x4)x > (u32x4)y);
}
i32x4 TESTFN i32x4_le_s(i32x4 x, i32x4 y) {
  return (i32x4)(x <= y);
}
i32x4 TESTFN i32x4_le_u(i32x4 x, i32x4 y) {
  return (i32x4)((u32x4)x <= (u32x4)y);
}
i32x4 TESTFN i32x4_ge_s(i32x4 x, i32x4 y) {
  return (i32x4)(x >= y);
}
i32x4  TESTFN i32x4_ge_u(i32x4 x, i32x4 y) {
  return (i32x4)((u32x4)x >= (u32x4)y);
}
i32x4 TESTFN f32x4_eq(f32x4 x, f32x4 y) {
  return (i32x4)(x == y);
}
i32x4 TESTFN f32x4_ne(f32x4 x, f32x4 y) {
  return (i32x4)(x != y);
}
i32x4 TESTFN f32x4_lt(f32x4 x, f32x4 y) {
  return (i32x4)(x < y);
}
i32x4 TESTFN f32x4_gt(f32x4 x, f32x4 y) {
  return (i32x4)(x > y);
}
i32x4 TESTFN f32x4_le(f32x4 x, f32x4 y) {
  return (i32x4)(x <= y);
}
i32x4 TESTFN f32x4_ge(f32x4 x, f32x4 y) {
  return (i32x4)(x >= y);
}
i64x2 TESTFN f64x2_eq(f64x2 x, f64x2 y) {
  return (i64x2)(x == y);
}
i64x2 TESTFN f64x2_ne(f64x2 x, f64x2 y) {
  return (i64x2)(x != y);
}
i64x2 TESTFN f64x2_lt(f64x2 x, f64x2 y) {
  return (i64x2)(x < y);
}
i64x2 TESTFN f64x2_gt(f64x2 x, f64x2 y) {
  return (i64x2)(x > y);
}
i64x2 TESTFN f64x2_le(f64x2 x, f64x2 y) {
  return (i64x2)(x <= y);
}
i64x2 TESTFN f64x2_ge(f64x2 x, f64x2 y) {
  return (i64x2)(x >= y);
}
v128 TESTFN v128_not(v128 vec) {
  return ~vec;
}
v128 TESTFN v128_and(v128 x, v128 y) {
  return x & y;
}
v128 TESTFN v128_or(v128 x, v128 y) {
  return x | y;
}
v128 TESTFN v128_xor(v128 x, v128 y) {
  return x ^ y;
}
v128 TESTFN v128_bitselect(v128 x, v128 y, v128 cond) {
  return (v128)__builtin_wasm_bitselect((i32x4)x, (i32x4)y, (i32x4)cond);
}
i8x16 TESTFN i8x16_neg(i8x16 vec) {
  return -vec;
}
int32_t TESTFN i8x16_any_true(i8x16 vec) {
  return __builtin_wasm_any_true_i8x16(vec);
}
int32_t TESTFN i8x16_all_true(i8x16 vec) {
  return __builtin_wasm_all_true_i8x16(vec);
}
// i8x16 TESTFN i8x16_shl(i8x16 vec, int32_t shift) {
//   return vec << shift;
// }
// i8x16 TESTFN i8x16_shr_s(i8x16 vec, int32_t shift) {
//   return vec >> shift;
// }
// i8x16 TESTFN i8x16_shr_u(i8x16 vec, int32_t shift) {
//   return (i8x16)((u8x16)vec >> shift);
// }
i8x16 TESTFN i8x16_add(i8x16 x, i8x16 y) {
  return x + y;
}
i8x16 TESTFN i8x16_add_saturate_s(i8x16 x, i8x16 y) {
  return __builtin_wasm_add_saturate_s_i8x16(x, y);
}
i8x16 TESTFN i8x16_add_saturate_u(i8x16 x, i8x16 y) {
  return __builtin_wasm_add_saturate_u_i8x16(x, y);
}
i8x16 TESTFN i8x16_sub(i8x16 x, i8x16 y) {
  return x - y;
}
i8x16 TESTFN i8x16_sub_saturate_s(i8x16 x, i8x16 y) {
  return __builtin_wasm_sub_saturate_s_i8x16(x, y);
}
i8x16 TESTFN i8x16_sub_saturate_u(i8x16 x, i8x16 y) {
  return __builtin_wasm_sub_saturate_u_i8x16(x, y);
}
i8x16 TESTFN i8x16_mul(i8x16 x, i8x16 y) {
  return x * y;
}
i16x8 TESTFN i16x8_neg(i16x8 vec) {
  return -vec;
}
int32_t TESTFN i16x8_any_true(i16x8 vec) {
  return __builtin_wasm_any_true_i16x8(vec);
}
int32_t TESTFN i16x8_all_true(i16x8 vec) {
  return __builtin_wasm_all_true_i16x8(vec);
}
// i16x8 TESTFN i16x8_shl(i16x8 vec, int32_t shift) {
//   return vec << shift;
// }
// i16x8 TESTFN i16x8_shr_s(i16x8 vec, int32_t shift) {
//   return vec >> shift;
// }
// i16x8 TESTFN i16x8_shr_u(i16x8 vec, int32_t shift) {
//   return (i16x8)((u16x8)vec >> shift);
// }
i16x8 TESTFN i16x8_add(i16x8 x, i16x8 y) {
  return x + y;
}
i16x8 TESTFN i16x8_add_saturate_s(i16x8 x, i16x8 y) {
  return __builtin_wasm_add_saturate_s_i16x8(x, y);
}
i16x8 TESTFN i16x8_add_saturate_u(i16x8 x, i16x8 y) {
  return __builtin_wasm_add_saturate_u_i16x8(x, y);
}
i16x8 TESTFN i16x8_sub(i16x8 x, i16x8 y) {
  return x - y;
}
i16x8 TESTFN i16x8_sub_saturate_s(i16x8 x, i16x8 y) {
  return __builtin_wasm_sub_saturate_s_i16x8(x, y);
}
i16x8 TESTFN i16x8_sub_saturate_u(i16x8 x, i16x8 y) {
  return __builtin_wasm_sub_saturate_u_i16x8(x, y);
}
i16x8 TESTFN i16x8_mul(i16x8 x, i16x8 y) {
  return x * y;
}
i32x4 TESTFN i32x4_neg(i32x4 vec) {
  return -vec;
}
int32_t TESTFN i32x4_any_true(i32x4 vec) {
  return __builtin_wasm_any_true_i32x4(vec);
}
int32_t TESTFN i32x4_all_true(i32x4 vec) {
  return __builtin_wasm_all_true_i32x4(vec);
}
// i32x4 TESTFN i32x4_shl(i32x4 vec, int32_t shift) {
//   return vec << shift;
// }
// i32x4 TESTFN i32x4_shr_s(i32x4 vec, int32_t shift) {
//   return vec >> shift;
// }
// i32x4 TESTFN i32x4_shr_u(i32x4 vec, int32_t shift) {
//   return (i32x4)((u32x4)vec >> shift);
// }
i32x4 TESTFN i32x4_add(i32x4 x, i32x4 y) {
  return x + y;
}
i32x4 TESTFN i32x4_sub(i32x4 x, i32x4 y) {
  return x - y;
}
i32x4 TESTFN i32x4_mul(i32x4 x, i32x4 y) {
  return x * y;
}
i64x2 TESTFN i64x2_neg(i64x2 vec) {
  return -vec;
}
// int32_t TESTFN i64x2_any_true(i64x2 vec) {
//   return __builtin_wasm_any_true_i64x2(vec);
// }
// int32_t TESTFN i64x2_all_true(i64x2 vec) {
//   return __builtin_wasm_all_true_i64x2(vec);
// }
// i64x2 TESTFN i64x2_shl(i64x2 vec, int32_t shift) {
//   return vec << shift;
// }
// i64x2 TESTFN i64x2_shr_s(i64x2 vec, int32_t shift) {
//   return vec >> shift;
// }
// i64x2 TESTFN i64x2_shr_u(i64x2 vec, int32_t shift) {
//   return (i64x2)((u64x2)vec >> shift);
// }
i64x2 TESTFN i64x2_add(i64x2 x, i64x2 y) {
  return x + y;
}
i64x2 TESTFN i64x2_sub(i64x2 x, i64x2 y) {
  return x - y;
}
f32x4 TESTFN f32x4_abs(f32x4 vec) {
  return __builtin_wasm_abs_f32x4(vec);
}
f32x4 TESTFN f32x4_neg(f32x4 vec) {
  return -vec;
}
f32x4 TESTFN f32x4_sqrt(f32x4 vec) {
  return __builtin_wasm_sqrt_f32x4(vec);
}
f32x4 TESTFN f32x4_add(f32x4 x, f32x4 y) {
  return x + y;
}
f32x4 TESTFN f32x4_sub(f32x4 x, f32x4 y) {
  return x - y;
}
f32x4 TESTFN f32x4_mul(f32x4 x, f32x4 y) {
  return x * y;
}
f32x4 TESTFN f32x4_div(f32x4 x, f32x4 y) {
  return x / y;
}
f32x4 TESTFN f32x4_min(f32x4 x, f32x4 y) {
  return __builtin_wasm_min_f32x4(x, y);
}
f32x4 TESTFN f32x4_max(f32x4 x, f32x4 y) {
  return __builtin_wasm_max_f32x4(x, y);
}
f64x2 TESTFN f64x2_abs(f64x2 vec) {
  return __builtin_wasm_abs_f64x2(vec);
}
f64x2 TESTFN f64x2_neg(f64x2 vec) {
  return -vec;
}
f64x2 TESTFN f64x2_sqrt(f64x2 vec) {
  return __builtin_wasm_sqrt_f64x2(vec);
}
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
f64x2 TESTFN f64x2_min(f64x2 x, f64x2 y) {
  return __builtin_wasm_min_f64x2(x, y);
}
f64x2 TESTFN f64x2_max(f64x2 x, f64x2 y) {
  return __builtin_wasm_max_f64x2(x, y);
}
i32x4 TESTFN i32x4_trunc_s_f32x4_sat(f32x4 vec) {
  return __builtin_wasm_trunc_saturate_s_i32x4_f32x4(vec);
}
i32x4 TESTFN i32x4_trunc_u_f32x4_sat(f32x4 vec) {
  return __builtin_wasm_trunc_saturate_u_i32x4_f32x4(vec);
}
// i64x2 TESTFN i64x2_trunc_s_f64x2_sat(f64x2 vec) {
//   return __builtin_wasm_trunc_saturate_s_i64x2_f64x2(vec);
// }
// i64x2 TESTFN i64x2_trunc_u_f64x2_sat(f64x2 vec) {
//   return __builtin_wasm_trunc_saturate_u_i64x2_f64x2(vec);
// }
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

#define expect_vec(_a, _b) ({                                           \
  i32x4 a = (i32x4)_a, b = (i32x4)_b;                                   \
  if (a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3]) {   \
    failures++;                                                         \
    fprintf(stderr, "line %d: expected {0x%08x, 0x%08x, 0x%08x, 0x%08x}, " \
            "got {0x%08x, 0x%08x, 0x%08x, 0x%08x}\n",                   \
            __LINE__, b[0], b[1], b[2], b[3], a[0], a[1], a[2], a[3]);  \
  }                                                                     \
})

#define formatter(x) _Generic((x),                      \
                              int64_t: "%ld",           \
                              int32_t: "%d",            \
                              uint32_t: "%d",           \
                              float: "%f",              \
                              double: "%f"              \
    )

#define err(x) fprintf(stderr, formatter(x), x)

#define expect_eq(_a, _b) ({                                            \
  __typeof__(_a) a = (_a), b = (_b);                                    \
  if (a != b) {                                                         \
    failures++;                                                         \
    fprintf(stderr, "line %d: expected ", __LINE__);                    \
    err(b);                                                             \
    fprintf(stderr, ", got ");                                          \
    err(a);                                                             \
    fprintf(stderr, "\n");                                              \
  }                                                                     \
})

int EMSCRIPTEN_KEEPALIVE main(int argc, char** argv) {
  {
    v128 vec = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    expect_vec(v128_load(&vec),
              ((v128){3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}));
    v128_store(&vec, (v128){7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7});
    expect_vec(v128_load(&vec),
              ((v128){7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}));
  }
  expect_vec(i32x4_const(), ((i32x4){1, 2, 3, 4}));
  // expect_vec(
  //   v128_shuffle_interleave_bytes(
  //     (v128){1, 0, 3, 0, 5, 0, 7, 0, 9, 0, 11, 0, 13, 0, 15, 0},
  //     (v128){0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16}
  //   ),
  //   ((v128){1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})
  // );
  // expect_vec(v128_shuffle_reverse_i32s((i32x4){1, 2, 3, 4}), ((i32x4){4, 3, 2, 1}));

  // i8x16 lane accesses
  expect_vec(i8x16_splat(5), ((i8x16){5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}));
  expect_vec(i8x16_splat(257), ((i8x16){1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}));
  expect_eq(i8x16_extract_lane_s_first((i8x16){255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), -1);
  expect_eq(i8x16_extract_lane_s_last((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255}), -1);
  expect_eq(i8x16_extract_lane_u_first((i8x16){255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), 255);
  expect_eq(i8x16_extract_lane_u_last((i8x16){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255}), 255);
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
  expect_eq(i16x8_extract_lane_u_first((i16x8){-1, 0, 0, 0, 0, 0, 0, 0}), 65535);
  expect_eq(i16x8_extract_lane_u_last((i16x8){0, 0, 0, 0, 0, 0, 0, -1}), 65535);
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
  expect_eq(i64x2_extract_lane_first((i64x2){-5, 0}), -5);
  expect_eq(i64x2_extract_lane_last((i64x2){0, -5}), -5);
  expect_vec(i64x2_replace_lane_first((i64x2){0, 0}, 53), ((i64x2){53, 0}));
  expect_vec(i64x2_replace_lane_last((i64x2){0, 0}, 53), ((i64x2){0, 53}));

  // f32x4 lane accesses
  expect_vec(f32x4_splat(-5), ((f32x4){-5, -5, -5, -5}));
  expect_eq(f32x4_extract_lane_first((f32x4){-5, 0, 0, 0}), -5);
  expect_eq(f32x4_extract_lane_last((f32x4){0, 0, 0, -5}), -5);
  expect_vec(f32x4_replace_lane_first((f32x4){0, 0, 0, 0}, 53), ((f32x4){53, 0, 0, 0}));
  expect_vec(f32x4_replace_lane_last((f32x4){0, 0, 0, 0}, 53), ((f32x4){0, 0, 0, 53}));

  // f64x2 lane accesses
  expect_vec(f64x2_splat(-5), ((f64x2){-5, -5}));
  expect_eq(f64x2_extract_lane_first((f64x2){-5, 0}), -5);
  expect_eq(f64x2_extract_lane_last((f64x2){0, -5}), -5);
  expect_vec(f64x2_replace_lane_first((f64x2){0, 0}, 53), ((f64x2){53, 0}));
  expect_vec(f64x2_replace_lane_last((f64x2){0, 0}, 53), ((f64x2){0, 53}));

  // i8x16 comparisons
  expect_vec(
    i8x16_eq(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){-1, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0})
  );
  expect_vec(
    i8x16_ne(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){0, -1, 0, -1, -1, -1, -1, -1, 0, -1, -1, 0, -1, -1, -1, -1})
  );
  expect_vec(
    i8x16_lt_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){0, 0, 0, -1, 0, -1, -1, 0, 0, 0, -1, 0, 0, -1, -1, 0})
  );
  expect_vec(
    i8x16_lt_u(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){0, -1, 0, 0, -1, -1, 0, -1, 0, -1, 0, 0, -1, -1, 0, -1})
  );
  expect_vec(
    i8x16_gt_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1})
  );
  expect_vec(
    i8x16_gt_u(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){0, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1, 0})
  );
  expect_vec(
    i8x16_le_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){-1, 0, -1, -1, 0, -1, -1, 0, -1, 0, -1, -1, 0, -1, -1, 0})
  );

  // expected {0x00ffffff, 0xff00ffff, 0xff00ffff, 0xff00ffff},
  //      got {0x0000ffff, 0xffffffff, 0x0000ffff, 0xffffffff}
  expect_vec(
    i8x16_le_u(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){-1, -1, -1, 0, -1, -1, 0, -1, -1, -1, 0, -1, -1, -1, 0, -1})
  );
  expect_vec(
    i8x16_ge_s(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){-1, -1, -1, 0, -1, 0, 0, -1, -1, -1, 0, -1, -1, 0, 0, -1})
  );
  // expected {0x00ffffff, 0xff00ffff, 0xff00ffff, 0xff00ffff},
  //      got {0x0000ffff, 0xffffffff, 0x0000ffff, 0xffffffff}
  expect_vec(
    i8x16_ge_u(
      (i8x16){0, 127, 13, 128,  1,  13, 129,  42, 0, 127, 255, 42,   1,  13, 129,  42},
      (i8x16){0, 255, 13, 42, 129, 127,   0, 128, 0, 255,  13, 42, 129, 127,   0, 128}
    ),
    ((i8x16){-1, 0, -1, -1, 0, 0, -1, 0, -1, 0, -1, -1, 0, 0, -1, 0})
  );

  // i16x8 comparisons
  expect_vec(
    i16x8_eq(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){-1, 0, 0, 0, 0, 0, 0, 0})
  );
  expect_vec(
    i16x8_ne(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){0, -1, -1, -1, -1, -1, -1, -1})
  );
  expect_vec(
    i16x8_lt_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){0, 0, 0, -1, 0, -1, 0, -1})
  );
  expect_vec(
    i16x8_lt_u(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){0, 0, 0, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_gt_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){0, -1, -1, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_gt_u(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){0, -1, -1, -1, 0, -1, 0, -1})
  );
  expect_vec(
    i16x8_le_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){-1, 0, 0, -1, 0, -1, 0, -1})
  );
  expect_vec(
    i16x8_le_u(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){-1, 0, 0, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_ge_s(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){-1, -1, -1, 0, -1, 0, -1, 0})
  );
  expect_vec(
    i16x8_ge_u(
      (i16x8){0, 32767, 13, -32768,      1, -32767,     42, -25536},
      (i16x8){0,    13,  1,  32767, -32767,     42, -25536,  32767}
    ),
    ((i16x8){-1, -1, -1, -1, 0, -1, 0, -1})
  );

  // i342x4 comparisons
  expect_vec(
    i32x4_eq((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){-1, 0, 0, 0})
  );
  expect_vec(
    i32x4_ne((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){0, -1, -1, -1})
  );
  expect_vec(
    i32x4_lt_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){0, -1, 0, -1})
  );
  expect_vec(
    i32x4_lt_u((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){0, 0, -1, -1})
  );
  expect_vec(
    i32x4_gt_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){0, 0, -1, 0})
  );
  expect_vec(
    i32x4_gt_u((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){0, -1, 0, 0})
  );
  expect_vec(
    i32x4_le_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){-1, -1, 0, -1})
  );
  expect_vec(
    i32x4_le_u((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){-1, 0, -1, -1})
  );
  expect_vec(
    i32x4_ge_s((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){-1, 0, -1, 0})
  );
  expect_vec(
    i32x4_ge_u((i32x4){0, -1, 53, -7}, (i32x4){0, 53, -7, -1}), ((i32x4){-1, -1, 0, 0})
  );

  // f32x4 comparisons
  expect_vec(
    f32x4_eq((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((i32x4){-1, 0, 0, 0})
  );
  expect_vec(
    f32x4_ne((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((i32x4){0, -1, -1, -1})
  );
  expect_vec(
    f32x4_lt((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((i32x4){0, -1, 0, -1})
  );
  expect_vec(
    f32x4_gt((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((i32x4){0, 0, -1, 0})
  );
  expect_vec(
    f32x4_le((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((i32x4){-1, -1, 0, -1})
  );
  expect_vec(
    f32x4_ge((f32x4){0, -1, 1, 0}, (f32x4){0, 0, -1, 1}), ((i32x4){-1, 0, -1, 0})
  );
  expect_vec(
    f32x4_eq((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((i32x4){0, 0, 0, -1})
  );
  expect_vec(
    f32x4_ne((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((i32x4){-1, -1, -1, 0})
  );
  expect_vec(
    f32x4_lt((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((i32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_gt((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((i32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_le((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((i32x4){0, 0, 0, -1})
  );
  expect_vec(
    f32x4_ge((f32x4){NAN, 0, NAN, INFINITY}, (f32x4){0, NAN, NAN, INFINITY}),
    ((i32x4){0, 0, 0, -1})
  );
  expect_vec(
    f32x4_eq((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((i32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_ne((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((i32x4){-1, -1, -1, -1})
  );
  expect_vec(
    f32x4_lt((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((i32x4){-1, -1, 0, 0})
  );
  expect_vec(
    f32x4_gt((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((i32x4){0, 0, 0, 0})
  );
  expect_vec(
    f32x4_le((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((i32x4){-1, -1, 0, 0})
  );
  expect_vec(
    f32x4_ge((f32x4){-INFINITY, 0, NAN, -INFINITY}, (f32x4){0, INFINITY, INFINITY, NAN}),
    ((i32x4){0, 0, 0, 0})
  );

  // f64x2 comparisons
  expect_vec(f64x2_eq((f64x2){0, 1}, (f64x2){0, 0}), ((i64x2){-1, 0}));
  expect_vec(f64x2_ne((f64x2){0, 1}, (f64x2){0, 0}), ((i64x2){0, -1}));
  expect_vec(f64x2_lt((f64x2){0, 1}, (f64x2){0, 0}), ((i64x2){0, 0}));
  expect_vec(f64x2_gt((f64x2){0, 1}, (f64x2){0, 0}), ((i64x2){0, -1}));
  expect_vec(f64x2_le((f64x2){0, 1}, (f64x2){0, 0}), ((i64x2){-1, 0}));
  expect_vec(f64x2_ge((f64x2){0, 1}, (f64x2){0, 0}), ((i64x2){-1, -1}));
  expect_vec(f64x2_eq((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((i64x2){0, 0}));
  expect_vec(f64x2_ne((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((i64x2){-1, -1}));
  expect_vec(f64x2_lt((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((i64x2){0, -1}));
  expect_vec(f64x2_gt((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((i64x2){0, 0}));
  expect_vec(f64x2_le((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((i64x2){0, -1}));
  expect_vec(f64x2_ge((f64x2){NAN, 0}, (f64x2){INFINITY, INFINITY}), ((i64x2){0, 0}));


  if (failures == 0) {
    printf("Success!\n");
  } else {
    printf("Failed :(\n");
  }
}
