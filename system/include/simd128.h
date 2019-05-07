/*
WebAssembly SIMD128 Intrinsics
*/

#include <stdint.h>

// User-facing types
typedef int8_t v128 __attribute__((__vector_size__(16)));
typedef int8_t i8x16 __attribute__((__vector_size__(16)));
typedef uint8_t u8x16 __attribute__((__vector_size__(16)));
typedef int16_t i16x8 __attribute__((__vector_size__(16)));
typedef uint16_t u16x8 __attribute__((__vector_size__(16)));
typedef int32_t i32x4 __attribute__((__vector_size__(16)));
typedef uint32_t u32x4 __attribute__((__vector_size__(16)));
typedef int64_t i64x2 __attribute__((__vector_size__(16)));
typedef uint64_t u64x2 __attribute__((__vector_size__(16)));
typedef float f32x4 __attribute__((__vector_size__(16)));
typedef double f64x2 __attribute__((__vector_size__(16)));

// Internal types
typedef char __v128 __attribute__((__vector_size__(16)));
typedef char  __i8x16 __attribute__((__vector_size__(16)));
typedef short __i16x8 __attribute__((__vector_size__(16)));
typedef int __i32x4 __attribute__((__vector_size__(16)));
typedef long long __i64x2 __attribute__((__vector_size__(16)));


#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))

// v128 wasm_v128_load(v128* mem)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_v128_load(i8x16* mem) {
  return __extension__(i8x16)(*mem);
}

// wasm_v128_store(v128 *mem, v128 a)
static __inline__ void __DEFAULT_FN_ATTRS wasm_v128_store(v128* mem, v128 a) {
  *(i8x16*)mem = a;
  return;
}

// v128 wasm_v128_constant(...)
static __inline__ v128 __DEFAULT_FN_ATTRS wasm_v128_const(int8_t c0,
  int8_t c1, int8_t c2, int8_t c3, int8_t c4, int8_t c5,
  int8_t c6,  int8_t c7,  int8_t c8,  int8_t c9,  int8_t c10,
  int8_t c11,  int8_t c12,  int8_t c13,  int8_t c14,  int8_t c15) {
  return (v128){c0, c1, c2,  c3,  c4,  c5,  c6,  c7,
                c8, c9, c10, c11, c12, c13, c14, c15};
}

// i8x16 wasm_i8x16_splat(int8_t a)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_splat(int8_t a) {
  return (i8x16){a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a};
}

// int8_t wasm_i8x16_extract_lane(i8x16 a, imm)
#define wasm_i8x16_extract_lane(a, b)  \
  (__builtin_wasm_extract_lane_s_i8x16((__i8x16)(a), b))

// int8_t wasm_u8x16_extract_lane(u8x16 a, imm)
#define wasm_u8x16_extract_lane(a, b) \
  (__builtin_wasm_extract_lane_u_i8x16((__i8x16)(a), b))

// i8x16 wasm_i8x16_replace_lane(i8x16 a, imm i, int8_t b)
#define wasm_i8x16_replace_lane(a, i, b) \
  ((i8x16)__builtin_wasm_replace_lane_i8x16((__i8x16)(a), i, b))

// i16x8 wasm_i16x8_splat(int16_t a)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_splat(int16_t a) {
  return (i16x8){a, a, a, a, a, a, a, a};
}

// int16_t wasm_i16x8_extract_lane(i16x8 a, imm)
#define wasm_i16x8_extract_lane(a, b) \
  (__builtin_wasm_extract_lane_s_i16x8((__i16x8)(a), b))

#ifdef __wasm_unimplemented_simd128__
// int16_t wasm_u16x8_extract_lane(u16x8 a, imm)
#define wasm_u16x8_extract_lane(a, b) \
  (__builtin_wasm_extract_lane_u_i16x8(a, b))
#endif

// i16x8 wasm_i16x8_replace_lane(i16x8 a, imm i, int16_t b)
#define wasm_i16x8_replace_lane(a, i, b) \
  ((i16x8)__builtin_wasm_replace_lane_i16x8((__i16x8)(a), i, b))

// i32x4 wasm_i32x4_splat(int32_t a)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_splat(int32_t a) {
  return (i32x4){a, a, a, a};
}

// int32_t wasm_i32x4_extract_lane(i32x4 a, imm)
#define wasm_i32x4_extract_lane(a, b) (__builtin_wasm_extract_lane_i32x4(a, b))

// i32x4 wasm_i32x4_replace_lane(i32x4 a, imm i, int32_t b)
#define wasm_i32x4_replace_lane(a, i, b) \
    ((i32x4)__builtin_wasm_replace_lane_i32x4((__i32x4)(a), i, b))

// i64x2 wasm_i64x2_splat(int64_t a)
static __inline__ i64x2 __DEFAULT_FN_ATTRS wasm_i64x2_splat(int64_t a) {
  return (i64x2){a, a};
}

// int64_t wasm_i64x2_extract_lane(i8x16, imm)
#define wasm_i64x2_extract_lane(a, b) (__builtin_wasm_extract_lane_i64x2(a, b))

// i8x16 wasm_i64x2_replace_lane(i8x16 a, imm i, int64_t b)
#define wasm_i64x2_replace_lane(a, i, b) \
  ((i64x2)__builtin_wasm_replace_lane_i64x2((__i64x2)(a), i, b))

// f32x4 wasm_f32x4_splat(float a)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_splat(float a) {
  return (f32x4){a, a, a, a};
}

// float wasm_f32x4_extract_lane(f32x4, imm)
#define wasm_f32x4_extract_lane(a, b) (__builtin_wasm_extract_lane_f32x4(a, b))

// f32x4 wasm_f32x4_replace_lane(f32x4 a, imm i, float b)
#define wasm_f32x4_replace_lane(a, i, b) \
  (__builtin_wasm_replace_lane_f32x4(a, i, b))

#ifdef __wasm_unimplemented_simd128__

// f64x2 wasm_f64x2_splat(double a)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_splat(double a) {
  return (f64x2){a, a};
}

// double __builtin_wasm_extract_lane_f64x2(f64x2, imm)
#define wasm_f64x2_extract_lane(a, b) (__builtin_wasm_extract_lane_f64x2(a, b))

// f64x2 wasm_f64x4_replace_lane(f64x2 a, imm i, double b)
#define wasm_f64x2_replace_lane(a, i, b) \
  (__builtin_wasm_replace_lane_f64x2(a, i, b))

#endif // __wasm_unimplemented_simd128__

// i8x16 wasm_i8x16_eq(i8x16 a, i8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_i8x16_eq(i8x16 a, i8x16 b) {
  return (u8x16)(a == b);
}

// i8x16 wasm_i8x16_ne(i8x16 a, i8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_i8x16_ne(i8x16 a, i8x16 b) {
  return (u8x16)(a != b);
}

// i8x16 wasm_i8x16_lt(i8x16 a, i8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_i8x16_lt(i8x16 a, i8x16 b) {
  return (u8x16)(a < b);
}

// i8x16 wasm_u8x16_lt(u8x16 a, u8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_lt(u8x16 a, u8x16 b) {
  return (u8x16)(a < b);
}

// i8x16 wasm_i8x16_gt(i8x16 a, i8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_i8x16_gt(i8x16 a, i8x16 b) {
  return (u8x16)(a > b);
}

// i8x16 wasm_u8x16_gt(u8x16 a, u8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_gt(u8x16 a, u8x16 b) {
  return (u8x16)(a > b);
}

// i8x16 wasm_i8x16_le(i8x16 a, i8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_i8x16_le(i8x16 a, i8x16 b) {
  return (u8x16)(a <= b);
}

// i8x16 wasm_i8x16_le(u8x16 a, u8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_le(u8x16 a, u8x16 b) {
  return (u8x16)(a <= b);
}

// i8x16 wasm_i8x16_ge(i8x16 a, i8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_i8x16_ge(i8x16 a, i8x16 b) {
  return (u8x16)(a >= b);
}

// i8x16 wasm_u8x16_ge(u8x16 a, u8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_ge(u8x16 a, u8x16 b) {
  return (u8x16)(a >= b);
}

// i16x8 wasm_i16x8_eq(i16x8 a, i16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_i16x8_eq(i16x8 a, i16x8 b) {
  return (u16x8)(a == b);
}

// i16x8 wasm_i16x8_ne(i16x8 a, i32x4 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_i16x8_ne(i16x8 a, i16x8 b) {
  return (u16x8)(a != b);
}
// i16x8 wasm_i16x8_lt(i16x8 a, i16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_i16x8_lt(i16x8 a, i16x8 b) {
  return (u16x8)(a < b);
}

// i16x8 wasm_u16x8_lt(u8x16 a, u8x16 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_lt(u16x8 a, u16x8 b) {
  return (u16x8)(a < b);
}

// i16x8 wasm_i16x8_gt(i16x8 a, i16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_i16x8_gt(i16x8 a, i16x8 b) {
  return (u16x8)(a > b);
}

// i16x8 wasm_u16x8_gt(u8x16 a, u8x16 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_gt(u16x8 a, u16x8 b) {
  return (u16x8)(a > b);
}

// i16x8 wasm_i16x8_le(i16x8 a, i16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_i16x8_le(i16x8 a, i16x8 b) {
  return (u16x8)(a <= b);
}

// i16x8 wasm_i16x8_le(u8x16 a, u8x16 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_le(u16x8 a, u16x8 b) {
  return (u16x8)(a <= b);
}

// i16x8 wasm_i16x8_ge(i16x8 a, i16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_i16x8_ge(i16x8 a, i16x8 b) {
  return (u16x8)(a >= b);
}

// i16x8 wasm_i16x8_ge(u16x8 a, u16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_ge(u16x8 a, u16x8 b) {
  return (u16x8)(a >= b);
}

// i32x4 wasm_i32x4_eq(i32x4 a, i32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_i32x4_eq(i32x4 a, i32x4 b) {
  return (u32x4)(a == b);
}

// i32x4 wasm_i32x4_ne(i32x4 a, i32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_i32x4_ne(i32x4 a, i32x4 b) {
  return (u32x4)(a != b);
}

// i32x4 wasm_i32x4_lt(i32x4 a, i32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_i32x4_lt(i32x4 a, i32x4 b) {
  return (u32x4)(a < b);
}

// u32x4 wasm_u32x4_lt(u32x4 a, u32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_u32x4_lt(u32x4 a, u32x4 b) {
  return (u32x4)(a < b);
}

// i32x4 wasm_i32x4_gt(i32x4 a, i32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_i32x4_gt(i32x4 a, i32x4 b) {
  return (u32x4)(a > b);
}

// i32x4 wasm_i32x4_gt(u32x4 a, u32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_u32x4_gt(u32x4 a, u32x4 b) {
  return (u32x4)(a > b);
}

// i32x4 wasm_i32x4_le(i32x4 a, i32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_i32x4_le(i32x4 a, i32x4 b) {
  return (u32x4)(a <= b);
}

// i32x4 wasm_u32x4_le(u32x4 a, u32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_u32x4_le(u32x4 a, u32x4 b) {
  return (u32x4)(a <= b);
}

// i32x4 wasm_i32x4_ge(i32x4 a, i32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_i32x4_ge(i32x4 a, i32x4 b) {
  return (u32x4)(a >= b);
}

// i32x4 wasm_u32x4_ge(u32x4 a, u32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_u32x4_ge(u32x4 a, u32x4 b) {
  return (u32x4)(a >= b);
}

// i32x4 wasm_f32x4_eq(f32x4 a f32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_f32x4_eq(f32x4 a, f32x4 b) {
  return (u32x4)(a == b);
}

// i32x4 wasm_f32x4_ne(f32x4 a, f32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_f32x4_ne(f32x4 a, f32x4 b) {
  return (u32x4)(a != b);
}

// i32x4 wasm_f32x4_lt(f32x4 a, f32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_f32x4_lt(f32x4 a, f32x4 b) {
  return (u32x4)(a < b);
}

// f32x4 wasm_f32x4_gt(f32x4 a, f32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_f32x4_gt(f32x4 a, f32x4 b) {
  return (u32x4)(a > b);
}

// i32x4 wasm_f32x4_le(f32x4 a, f32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_f32x4_le(f32x4 a, f32x4 b) {
  return (u32x4)(a <= b);
}

// i32x4 wasm_f32x4_ge(f32x4 a, f32x4 b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_f32x4_ge(f32x4 a, f32x4 b) {
  return (u32x4)(a >= b);
}

#ifdef __wasm_unimplemented_simd128__

// i64x2 wasm_f64x2_eq(f64x2 a, f64x2 b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_f64x2_eq(f64x2 a, f64x2 b) {
  return (u64x2)(a == b);
}

// i64x2 wasm_f64x2_ne(f64x2 a, f64x2 b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_f64x2_ne(f64x2 a, f64x2 b) {
  return (u64x2)(a != b);
}

// i64x2 wasm_f64x2_lt(f64x2 a, f64x2 b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_f64x2_lt(f64x2 a, f64x2 b) {
  return (u64x2)(a < b);
}

// i64x2 wasm_f64x2_gt(f64x2 a, f64x2 b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_f64x2_gt(f64x2 a, f64x2 b) {
  return (u64x2)(a > b);
}

// i64x2 wasm_f64x2_le(f64x2 a, f64x2 b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_f64x2_le(f64x2 a, f64x2 b) {
  return (u64x2)(a <= b);
}

// i64x2 wasm_f64x2_ge(f64x2 a, f64x2 b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_f64x2_ge(f64x2 a, f64x2 b) {
  return (u64x2)(a >= b);
}

#endif // __wasm_unimplemented_simd128__

// v128 wasm_v128_not(v128 a)
static __inline__ v128 __DEFAULT_FN_ATTRS wasm_v128_not(v128 a) {
  return ~a;
}

// v128 wasm_v128_and(v128 a, v128 b)
static __inline__ v128 __DEFAULT_FN_ATTRS wasm_v128_and(v128 a, v128 b) {
  return a & b;
}

// v128 wasm_v128_or(v128 a, v128 b)
static __inline__ v128 __DEFAULT_FN_ATTRS wasm_v128_or(v128 a, v128 b) {
  return a | b;
}

// v128 wasm_v128_xor(v128 a, v128 b)
static __inline__ v128 __DEFAULT_FN_ATTRS wasm_v128_xor(v128 a, v128 b) {
  return a ^ b;
}

// v128 wasm_v128_bitselect(v128 a, v128 b, v128 c)
static __inline__ v128 __DEFAULT_FN_ATTRS wasm_v128_bitselect(v128 a, v128 b, v128 c) {
  return (v128)__builtin_wasm_bitselect((__i32x4)a, (__i32x4)b, (__i32x4)c);
}

// i8x16 wasm_i8x16_neg(i8x16 a)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_neg(i8x16 a) {
  return -a;
}

// bool wasm_i8x16_any_true(i8x16 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i8x16_any_true(i8x16 a) {
  return __builtin_wasm_any_true_i8x16((__i8x16)a);
}

// bool wasm_i8x16_all_true(i8x16 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i8x16_all_true(i8x16 a) {
  return __builtin_wasm_all_true_i8x16((__i8x16)a);
}

// i8x16 wasm_i8x16_shl(i8x16 a, int32_t b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_shl(i8x16 a, int32_t b) {
  return a << b;
}

// i8x16 wasm_i8x64_shr(i8x16 a, int32_t b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_shr(i8x16 a, int32_t b) {
  return a >> b;
}

// u8x16 wasm_u8x16_shr(u8x16 a int32_t b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_shr(u8x16 a, int32_t b) {
  return a >> b;
}

// i8x16 wasm_i8x16_add(i8x16 a i8x16 b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_add(i8x16 a, i8x16 b) {
  return a + b;
}

// i8x16 wasm_add_saturate(i8x16 a, i8x16 b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_add_saturate(i8x16 a, i8x16 b) {
  return (i8x16) __builtin_wasm_add_saturate_s_i8x16((__i8x16)a, (__i8x16)b);
}

// u8x16 wasm_add_saturate(u8x16 a, u8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_add_saturate(u8x16 a, u8x16 b) {
  return (u8x16)__builtin_wasm_add_saturate_u_i8x16((__i8x16)a, (__i8x16)b);
}

// i8x16 wasm_i8x16_sub(i8x16 a, i8x16 b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_sub(i8x16 a, i8x16 b) {
  return a - b;
}

// i8x16 wasm_sub_saturate(i8x16 a, i8x16 b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_sub_saturate(i8x16 a, i8x16 b) {
  return (i8x16)__builtin_wasm_sub_saturate_s_i8x16((__i8x16)a, (__i8x16)b);
}

// u8x16 wasm_sub_saturate(u8x16 a, u8x16 b)
static __inline__ u8x16 __DEFAULT_FN_ATTRS wasm_u8x16_sub_saturate(u8x16 a, u8x16 b) {
  return (u8x16)__builtin_wasm_sub_saturate_u_i8x16((__i8x16)a, (__i8x16)b);
}

// i8x16 wasm_i8x16_mul(i8x16 a i8x16 b)
static __inline__ i8x16 __DEFAULT_FN_ATTRS wasm_i8x16_mul(i8x16 a, i8x16 b) {
  return a * b;
}

// i16x8 wasm_i16x8_neg(i16x8 a)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_neg(i16x8 a) {
  return -a;
}

// bool wasm_i16x8_any_true(i16x8 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i16x8_any_true(i16x8 a) {
  return __builtin_wasm_any_true_i16x8((__i16x8)a);
}

// bool wasm_i16x8_all_true(i16x8 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i16x8_all_true(i16x8 a) {
  return __builtin_wasm_all_true_i16x8((__i16x8)a);
}

// i16x8 wasm_i16x8_shl(i16x8 a, int32_t b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_shl(i16x8 a, int32_t b) {
  return a << b;
}

// i16x8 wasm_i16x8_shr(i16x8 a, int32_t b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_shr(i16x8 a, int32_t b) {
  return a >> b;
}

// u16x8 wasm_u16x8_shr(u16x8 a, int32_t b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_shr(u16x8 a, int32_t b) {
  return a >> b;
}

// i16x8 wasm_i16x8_add(i16x8 a i16x8 b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_add(i16x8 a, i16x8 b) {
  return a + b;
}

// i16x8 wasm_add_saturate(i16x8 a, i16x8 b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_add_saturate(i16x8 a, i16x8 b) {
  return (i16x8) __builtin_wasm_add_saturate_s_i16x8((__i16x8)a, (__i16x8)b);
}

// u16x8 wasm_add_saturate(u16x8 a, u16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_add_saturate(u16x8 a, u16x8 b) {
  return (u16x8)__builtin_wasm_add_saturate_u_i16x8((__i16x8)a, (__i16x8)b);
}

// i16x8 wasm_i16x8_sub(i16x8 a i16x8 b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_sub(i16x8 a, i16x8 b) {
  return a - b;
}

// i16x8 wasm_sub_saturate(i16x8 a, i16x8 b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_sub_saturate(i16x8 a, i16x8 b) {
  return (i16x8)__builtin_wasm_sub_saturate_s_i16x8((__i16x8)a, (__i16x8)b);
}

// u16x8 wasm_sub_saturate(u16x8 a, u16x8 b)
static __inline__ u16x8 __DEFAULT_FN_ATTRS wasm_u16x8_sub_saturate(u16x8 a, u16x8 b) {
  return (u16x8)__builtin_wasm_sub_saturate_u_i16x8((__i16x8)a, (__i16x8)b);
}

// i16x8 wasm_i16x8_mul(i16x8 a i16x8 b)
static __inline__ i16x8 __DEFAULT_FN_ATTRS wasm_i16x8_mul(i16x8 a, i16x8 b) {
  return a * b;
}

// i32x4 wasm_i32x4_neg(i32x4 a)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_neg(i32x4 a) {
  return -a;
}

// bool wasm_i32x4_any_true(i32x4 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i32x4_any_true(i32x4 a) {
  return __builtin_wasm_any_true_i32x4((__i32x4)a);
}

// bool wasm_i32x4_all_true(i32x4 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i32x4_all_true(i32x4 a) {
  return __builtin_wasm_all_true_i32x4((__i32x4)a);
}

// i32x4 wasm_i32x4_shl(i32x4 a, int32_t b)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_shl(i32x4 a, int32_t b) {
  return a << b;
}

// i32x4 wasm_i32x4_shr(i32x4 a, int32_t b)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_shr(i32x4 a, int32_t b) {
  return a >> b;
}

// u32x4 wasm_u32x4_shr(u32x4 a, int32_t b)
static __inline__ u32x4 __DEFAULT_FN_ATTRS wasm_u32x4_shr(u32x4 a, int32_t b) {
  return a >> b;
}

// i32x4 wasm_i32x4_add(i32x4 a i32x4 b)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_add(i32x4 a, i32x4 b) {
  return a + b;
}

// i32x4 wasm_i32x4_sub(i32x4 a i32x4 b)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_sub(i32x4 a, i32x4 b) {
  return a - b;
}

// i32x4 wasm_i32x4_mul(i32x4 a i32x4 b)
static __inline__ i32x4 __DEFAULT_FN_ATTRS wasm_i32x4_mul(i32x4 a, i32x4 b) {
  return a * b;
}

// i64x2 wasm_i64x2_neg(i64x2 a)
static __inline__ i64x2 __DEFAULT_FN_ATTRS wasm_i64x2_neg(i64x2 a) {
  return -a;
}

// bool wasm_i64x2_any_true(i64x2 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i64x2_any_true(i64x2 a) {
  return __builtin_wasm_any_true_i64x2((__i64x2)a);
}

// bool wasm_i64x2_all_true(i64x2 a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i64x2_all_true(i64x2 a) {
  return __builtin_wasm_all_true_i64x2((__i64x2)a);
}

// i64x2 wasm_i64x2_shl(i64x2 a, int32_t b)
static __inline__ i64x2 __DEFAULT_FN_ATTRS wasm_i64x2_shl(i64x2 a, int32_t b) {
  return a << b;
}

// i64x2 wasm_i64x2_shr(i64x2 a, int32_t b)
static __inline__ i64x2 __DEFAULT_FN_ATTRS wasm_i64x2_shr(i64x2 a, int32_t b) {
  return a >> b;
}

// u64x2 wasm_u64x2_shr_u(u64x2 a, int32_t b)
static __inline__ u64x2 __DEFAULT_FN_ATTRS wasm_u64x2_shr(u64x2 a, int32_t b) {
  return a >> b;
}

// i8x16 wasm_i64x2_add(i8x16 a i8x16 b)
static __inline__ i64x2 __DEFAULT_FN_ATTRS wasm_i64x2_add(i64x2 a, i64x2 b) {
  return a + b;
}

// i64x2 wasm_i64x2_sub(i64x2 a i64x2 b)
static __inline__ i64x2 __DEFAULT_FN_ATTRS wasm_i64x2_sub(i64x2 a, i64x2 b) {
  return a - b;
}

// f32x4  wasm_f32x4_abs(f32x4 a)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_abs(f32x4 a) {
  return __builtin_wasm_abs_f32x4(a);
}

// f32x4 wasm_f32x4_neg(f32x4 a)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_neg(f32x4 a) {
  return -a;
}

// f32x4 wasm_f32x4_sqrt(f32x4 a)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_sqrt(f32x4 a) {
  return __builtin_wasm_sqrt_f32x4(a);
}

// f32x4 wasm_f32x4_add(f32x4 a f32x4 b)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_add(f32x4 a, f32x4 b) {
  return a + b;
}

// f32x4 wasm_f32x4_sub(f32x4 a f32x4 b)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_sub(f32x4 a, f32x4 b) {
  return a - b;
}

// f32x4 wasm_f32x4_mul(f32x4 a f32x4 b)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_mul(f32x4 a, f32x4 b) {
  return a * b;
}

// f32x4 wasm_f32x4_div(f32x4 a f32x4 b)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_div(f32x4 a, f32x4 b) {
  return a / b;
}

// f32x4 wasm_f32x4_min(f32x4 a f32x4 b)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_min(f32x4 a, f32x4 b) {
  return __builtin_wasm_min_f32x4(a, b);
}

// f32x4 wasm_f32x4_max(f32x4 a f32x4 b)
static __inline__ f32x4 __DEFAULT_FN_ATTRS wasm_f32x4_max(f32x4 a, f32x4 b) {
  return __builtin_wasm_max_f32x4(a, b);
}

#ifdef __wasm_unimplemented_simd128__

// f64x2  wasm_f64x2_abs(f64x2 a)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_abs(f64x2 a) {
  return __builtin_wasm_abs_f64x2(a);
}

// f64x2 wasm_f64x2_neg(f64x2 a)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_neg(f64x2 a) {
  return -a;
}

// f64x2  wasm_f64x2_sqrt(f64x2 a)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_sqrt(f64x2 a) {
  return __builtin_wasm_sqrt_f64x2(a);
}

// f64x2 wasm_f64x2_add(f64x2 a f64x2 b)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_add(f64x2 a, f64x2 b) {
  return a + b;
}

// f64x2 wasm_f64x2_sub(f64x2 a f64x2 b)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_sub(f64x2 a, f64x2 b) {
  return a - b;
}

// f64x2 wasm_f64x2_mul(f64x2 a f64x2 b)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_mul(f64x2 a, f64x2 b) {
  return a * b;
}

// f64x2 wasm_f64x2_div(f64x2 a f64x2 b)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_div(f64x2 a, f64x2 b) {
  return a / b;
}

// f64x2 wasm_f64x2_min(f64x2 a f64x2 b)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_min(f64x2 a, f64x2 b) {
  return __builtin_wasm_min_f64x2(a, b);
}

// f64x2 wasm_f64x2_max(f64x2 a f64x2 b)
static __inline__ f64x2 __DEFAULT_FN_ATTRS wasm_f64x2_max(f64x2 a, f64x2 b) {
  return __builtin_wasm_max_f64x2(a, b);
}

#endif // __wasm_unimplemented_simd128__

// // f32x4 wasm_convert_f32x4_i32x4(i32x4 a)
// #define wasm_convert_f32x4_i32x4(v) (__builtin_convertvector(v, f32x4))

// // f32x4 wasm_convert_f32x4_u32x4(u32x4 a)
// #define wasm_convert_f32x4_u32x4(v) (__builtin_convertvector(v, f32x4))

// // f64x2 wasm_convert_f64x2_i64x2(i64x2 a)
// #define wasm_convert_f64x2_i64x2(v) (__builtin_convertvector(v, f64x2))

// // f64x2 wasm_convert_f64x2_u64x2(u64x2 a)
// #define wasm_convert_f64x2_u64x2(v) (__builtin_convertvector(v, f64x2))

// not sure how this should work with variable input
// #define wasm_i8x16_shuffle(a, b) \
//  (__builtin_shufflevector(a, b, 0, 1, 2, 3, 4, 5, 6, 7))
