/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/*
 * WebAssembly SIMD128 Intrinsics
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// User-facing type
typedef int32_t v128_t __attribute__((__vector_size__(16), __aligned__(16)));

// Internal types determined by clang builtin definitions
typedef int32_t __v128_u __attribute__((__vector_size__(16), __aligned__(1)));
typedef char  __i8x16 __attribute__((__vector_size__(16), __aligned__(16)));
typedef unsigned char __u8x16 __attribute__((__vector_size__(16), __aligned__(16)));
typedef short __i16x8 __attribute__((__vector_size__(16), __aligned__(16)));
typedef unsigned short __u16x8 __attribute__((__vector_size__(16), __aligned__(16)));
typedef int __i32x4 __attribute__((__vector_size__(16), __aligned__(16)));
typedef unsigned int __u32x4 __attribute__((__vector_size__(16), __aligned__(16)));
typedef long long __i64x2 __attribute__((__vector_size__(16), __aligned__(16)));
typedef unsigned long long __u64x2 __attribute__((__vector_size__(16), __aligned__(16)));
typedef float __f32x4 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __f64x2 __attribute__((__vector_size__(16), __aligned__(16)));

#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("simd128"), __min_vector_width__(128)))

#ifdef __cplusplus
#include <type_traits>
#define __SAME_TYPE(t1, t2) (std::is_same<t1, t2>::value)
#else
#define __SAME_TYPE(t1, t2) (__builtin_types_compatible_p(t1, t2))
#endif

#define __REQUIRE_CONSTANT(e, ty, msg) _Static_assert(__builtin_constant_p(e) && __SAME_TYPE(__typeof__(e), ty), msg)

// v128 wasm_v128_load(void* mem)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_v128_load(const void* __mem) {
  // UB-free unaligned access copied from xmmintrin.h
  struct __wasm_v128_load_struct {
    __v128_u __v;
  } __attribute__((__packed__, __may_alias__));
  return ((const struct __wasm_v128_load_struct*)__mem)->__v;
}

// wasm_v128_store(void* mem, v128 a)
static __inline__ void __DEFAULT_FN_ATTRS wasm_v128_store(void* __mem, v128_t __a) {
  // UB-free unaligned access copied from xmmintrin.h
  struct __wasm_v128_store_struct {
    __v128_u __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __wasm_v128_store_struct*)__mem)->__v = __a;
}

// wasm_i8x16_make(...)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_make(int8_t c0, int8_t c1, int8_t c2, int8_t c3, int8_t c4, int8_t c5, int8_t c6, int8_t c7, int8_t c8, int8_t c9, int8_t c10, int8_t c11, int8_t c12, int8_t c13, int8_t c14, int8_t c15) {
  return (v128_t)(__i8x16){c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15};
}

// wasm_i16x8_make(...)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_make(int16_t c0, int16_t c1, int16_t c2, int16_t c3, int16_t c4, int16_t c5, int16_t c6, int16_t c7) {
  return (v128_t)(__i16x8){c0, c1, c2, c3, c4, c5, c6, c7};
}

// wasm_i32x4_make(...)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_make(int32_t c0, int32_t c1, int32_t c2, int32_t c3) {
  return (v128_t)(__i32x4){c0, c1, c2, c3};
}

// wasm_f32x4_make(...)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_make(float c0, float c1, float c2, float c3) {
  return (v128_t)(__f32x4){c0, c1, c2, c3};
}

#ifdef __wasm_unimplemented_simd128__

// wasm_i64x2_make(...)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_make(int64_t c0, int64_t c1) {
  return (v128_t)(__i64x2){c0, c1};
}

// wasm_f64x2_make(...)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_make(double c0, double c1) {
  return (v128_t)(__f64x2){c0, c1};
}

#endif // __wasm_unimplemented_simd128__

// v128_t wasm_i8x16_constant(...)
#define wasm_i8x16_const(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
  __extension__({                                                       \
      __REQUIRE_CONSTANT(c0, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c1, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c2, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c3, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c4, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c5, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c6, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c7, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c8, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c9, int8_t, "expected constant int8_t");       \
      __REQUIRE_CONSTANT(c10, int8_t, "expected constant int8_t");      \
      __REQUIRE_CONSTANT(c11, int8_t, "expected constant int8_t");      \
      __REQUIRE_CONSTANT(c12, int8_t, "expected constant int8_t");      \
      __REQUIRE_CONSTANT(c13, int8_t, "expected constant int8_t");      \
      __REQUIRE_CONSTANT(c14, int8_t, "expected constant int8_t");      \
      __REQUIRE_CONSTANT(c15, int8_t, "expected constant int8_t");      \
      (v128_t)(__i8x16){c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15}; \
    })

// v128_t wasm_i16x8_constant(...)
#define wasm_i16x8_const(c0, c1, c2, c3, c4, c5, c6, c7) \
  __extension__({                                                       \
      __REQUIRE_CONSTANT(c0, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c1, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c2, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c3, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c4, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c5, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c6, int16_t, "expected constant int16_t");     \
      __REQUIRE_CONSTANT(c7, int16_t, "expected constant int16_t");     \
      (v128_t)(__i16x8){c0, c1, c2, c3, c4, c5, c6, c7}; \
    })

// v128_t wasm_i32x4_constant(...)
#define wasm_i32x4_const(c0, c1, c2, c3) \
  __extension__({                                                       \
      __REQUIRE_CONSTANT(c0, int32_t, "expected constant int32_t");     \
      __REQUIRE_CONSTANT(c1, int32_t, "expected constant int32_t");     \
      __REQUIRE_CONSTANT(c2, int32_t, "expected constant int32_t");     \
      __REQUIRE_CONSTANT(c3, int32_t, "expected constant int32_t");     \
      (v128_t)(__i32x4){c0, c1, c2, c3};                                \
    })

// v128_t wasm_f32x4_constant(...)
#define wasm_f32x4_const(c0, c1, c2, c3)                        \
  __extension__({                                               \
      __REQUIRE_CONSTANT(c0, float, "expected constant float"); \
      __REQUIRE_CONSTANT(c1, float, "expected constant float"); \
      __REQUIRE_CONSTANT(c2, float, "expected constant float"); \
      __REQUIRE_CONSTANT(c3, float, "expected constant float"); \
      (v128_t)(__f32x4){c0, c1, c2, c3};                        \
    })

#ifdef __wasm_unimplemented_simd128__

// v128_t wasm_i64x2_constant(...)
#define wasm_i64x2_const(c0, c1)                                        \
  __extension__({                                                       \
      __REQUIRE_CONSTANT(c0, int64_t, "expected constant int64_t");     \
      __REQUIRE_CONSTANT(c1, int64_t, "expected constant int64_t");     \
      (v128_t)(__i64x2){c0, c1};                                        \
    })

// v128_t wasm_f64x2_constant(...)
#define wasm_f64x2_const(c0, c1)                                        \
  __extension__({                                                       \
      __REQUIRE_CONSTANT(c0, double, "expected constant double");     \
      __REQUIRE_CONSTANT(c1, double, "expected constant double");     \
      (v128_t)(__f64x2){c0, c1};                                        \
    })

#endif // __wasm_unimplemented_sidm128__

// v128_t wasm_i8x16_splat(int8_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_splat(int8_t a) {
  return (v128_t)(__i8x16){a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a};
}

// int8_t wasm_i8x16_extract_lane(v128_t a, imm i)
#define wasm_i8x16_extract_lane(a, i) (__builtin_wasm_extract_lane_s_i8x16((__i8x16)(a), i))

// int8_t wasm_u8x16_extract_lane(v128_t a, imm i)
#define wasm_u8x16_extract_lane(a, i) (__builtin_wasm_extract_lane_u_i8x16((__i8x16)(a), i))

// v128_t wasm_i8x16_replace_lane(v128_t a, imm i, int8_t b)
#define wasm_i8x16_replace_lane(a, i, b)                                                           \
  ((v128_t)__builtin_wasm_replace_lane_i8x16((__i8x16)(a), i, b))

// v128_t wasm_i16x8_splat(int16_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_splat(int16_t a) {
  return (v128_t)(__i16x8){a, a, a, a, a, a, a, a};
}

// int16_t wasm_i16x8_extract_lane(v128_t a, imm i)
#define wasm_i16x8_extract_lane(a, i) (__builtin_wasm_extract_lane_s_i16x8((__i16x8)(a), i))

#ifdef __wasm_unimplemented_simd128__

// int16_t wasm_u16x8_extract_lane(v128_t a, imm i)
#define wasm_u16x8_extract_lane(a, i) (__builtin_wasm_extract_lane_u_i16x8((__i16x8)(a), i))

#endif // __wasm_unimplemented_simd128__

// v128_t wasm_i16x8_replace_lane(v128_t a, imm i, int16_t b)
#define wasm_i16x8_replace_lane(a, i, b)                                                           \
  ((v128_t)__builtin_wasm_replace_lane_i16x8((__i16x8)(a), i, b))

// v128_t wasm_i32x4_splat(int32_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_splat(int32_t a) {
  return (v128_t)(__i32x4){a, a, a, a};
}

// int32_t wasm_i32x4_extract_lane(v128_t a, imm i)
#define wasm_i32x4_extract_lane(a, i) (__builtin_wasm_extract_lane_i32x4((__i32x4)(a), i))

// v128_t wasm_i32x4_replace_lane(v128_t a, imm i, int32_t b)
#define wasm_i32x4_replace_lane(a, i, b)                                                           \
  ((v128_t)__builtin_wasm_replace_lane_i32x4((__i32x4)(a), i, b))

#ifdef __wasm_unimplemented_simd128__

// v128_t wasm_i64x2_splat(int64_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_splat(int64_t a) {
  return (v128_t)(__i64x2){a, a};
}

#endif // __wasm_unimplemented_simd128__

// int64_t wasm_i64x2_extract_lane(v128_t a, imm i)
#define wasm_i64x2_extract_lane(a, i) (__builtin_wasm_extract_lane_i64x2((__i64x2)(a), i))

// v128_t wasm_i64x2_replace_lane(v128_t a, imm i, int64_t b)
#define wasm_i64x2_replace_lane(a, i, b)                                                           \
  ((v128_t)__builtin_wasm_replace_lane_i64x2((__i64x2)(a), i, b))

// v128_t wasm_f32x4_splat(float a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_splat(float a) {
  return (v128_t)(__f32x4){a, a, a, a};
}

// float wasm_f32x4_extract_lane(v128_t a, imm i)
#define wasm_f32x4_extract_lane(a, i) (__builtin_wasm_extract_lane_f32x4((__f32x4)(a), i))

// v128_t wasm_f32x4_replace_lane(v128_t a, imm i, float b)
#define wasm_f32x4_replace_lane(a, i, b)                                                           \
  ((v128_t)__builtin_wasm_replace_lane_f32x4((__f32x4)(a), i, b))

#ifdef __wasm_unimplemented_simd128__

// v128_t wasm_f64x2_splat(double a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_splat(double a) {
  return (v128_t)(__f64x2){a, a};
}

// double __builtin_wasm_extract_lane_f64x2(v128_t a, imm i)
#define wasm_f64x2_extract_lane(a, i) (__builtin_wasm_extract_lane_f64x2((__f64x2)(a), i))

// v128_t wasm_f64x4_replace_lane(v128_t a, imm i, double b)
#define wasm_f64x2_replace_lane(a, i, b)                                                           \
  ((v128_t)__builtin_wasm_replace_lane_f64x2((__f64x2)(a), i, b))

#endif // __wasm_unimplemented_simd128__

// v128_t wasm_i8x16_eq(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_eq(v128_t a, v128_t b) {
  return (v128_t)((__i8x16)a == (__i8x16)b);
}

// v128_t wasm_i8x16_ne(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_ne(v128_t a, v128_t b) {
  return (v128_t)((__i8x16)a != (__i8x16)b);
}

// v128_t wasm_i8x16_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_lt(v128_t a, v128_t b) {
  return (v128_t)((__i8x16)a < (__i8x16)b);
}

// v128_t wasm_u8x16_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_lt(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a < (__u8x16)b);
}

// v128_t wasm_i8x16_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_gt(v128_t a, v128_t b) {
  return (v128_t)((__i8x16)a > (__i8x16)b);
}

// v128_t wasm_u8x16_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_gt(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a > (__u8x16)b);
}

// v128_t wasm_i8x16_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_le(v128_t a, v128_t b) {
  return (v128_t)((__i8x16)a <= (__i8x16)b);
}

// v128_t wasm_i8x16_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_le(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a <= (__u8x16)b);
}

// v128_t wasm_i8x16_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_ge(v128_t a, v128_t b) {
  return (v128_t)((__i8x16)a >= (__i8x16)b);
}

// v128_t wasm_u8x16_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_ge(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a >= (__u8x16)b);
}

// v128_t wasm_i16x8_eq(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_eq(v128_t a, v128_t b) {
  return (v128_t)((__i16x8)a == (__i16x8)b);
}

// v128_t wasm_i16x8_ne(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_ne(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a != (__u16x8)b);
}

// v128_t wasm_i16x8_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_lt(v128_t a, v128_t b) {
  return (v128_t)((__i16x8)a < (__i16x8)b);
}

// v128_t wasm_u16x8_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_lt(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a < (__u16x8)b);
}

// v128_t wasm_i16x8_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_gt(v128_t a, v128_t b) {
  return (v128_t)((__i16x8)a > (__i16x8)b);
}

// v128_t wasm_u16x8_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_gt(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a > (__u16x8)b);
}

// v128_t wasm_i16x8_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_le(v128_t a, v128_t b) {
  return (v128_t)((__i16x8)a <= (__i16x8)b);
}

// v128_t wasm_i16x8_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_le(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a <= (__u16x8)b);
}

// v128_t wasm_i16x8_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_ge(v128_t a, v128_t b) {
  return (v128_t)((__i16x8)a >= (__i16x8)b);
}

// v128_t wasm_i16x8_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_ge(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a >= (__u16x8)b);
}

// v128_t wasm_i32x4_eq(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_eq(v128_t a, v128_t b) {
  return (v128_t)((__i32x4)a == (__i32x4)b);
}

// v128_t wasm_i32x4_ne(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_ne(v128_t a, v128_t b) {
  return (v128_t)((__i32x4)a != (__i32x4)b);
}

// v128_t wasm_i32x4_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_lt(v128_t a, v128_t b) {
  return (v128_t)((__i32x4)a < (__i32x4)b);
}

// v128_t wasm_u32x4_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u32x4_lt(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a < (__u32x4)b);
}

// v128_t wasm_i32x4_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_gt(v128_t a, v128_t b) {
  return (v128_t)((__i32x4)a > (__i32x4)b);
}

// v128_t wasm_i32x4_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u32x4_gt(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a > (__u32x4)b);
}

// v128_t wasm_i32x4_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_le(v128_t a, v128_t b) {
  return (v128_t)((__i32x4)a <= (__i32x4)b);
}

// v128_t wasm_u32x4_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u32x4_le(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a <= (__u32x4)b);
}

// v128_t wasm_i32x4_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_ge(v128_t a, v128_t b) {
  return (v128_t)((__i32x4)a >= (__i32x4)b);
}

// v128_t wasm_u32x4_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u32x4_ge(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a >= (__u32x4)b);
}

// v128_t wasm_f32x4_eq(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_eq(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a == (__f32x4)b);
}

// v128_t wasm_f32x4_ne(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_ne(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a != (__f32x4)b);
}

// v128_t wasm_f32x4_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_lt(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a < (__f32x4)b);
}

// v128_t wasm_f32x4_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_gt(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a > (__f32x4)b);
}

// v128_t wasm_f32x4_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_le(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a <= (__f32x4)b);
}

// v128_t wasm_f32x4_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_ge(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a >= (__f32x4)b);
}

#ifdef __wasm_unimplemented_simd128__

// v128_t wasm_f64x2_eq(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_eq(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a == (__f64x2)b);
}

// v128_t wasm_f64x2_ne(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_ne(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a != (__f64x2)b);
}

// v128_t wasm_f64x2_lt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_lt(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a < (__f64x2)b);
}

// v128_t wasm_f64x2_gt(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_gt(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a > (__f64x2)b);
}

// v128_t wasm_f64x2_le(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_le(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a <= (__f64x2)b);
}

// v128_t wasm_f64x2_ge(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_ge(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a >= (__f64x2)b);
}

#endif // __wasm_unimplemented_simd128__

// v128_t wasm_v128_not(v128 a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_v128_not(v128_t a) { return ~a; }

// v128_t wasm_v128_and(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_v128_and(v128_t a, v128_t b) { return a & b; }

// v128_t wasm_v128_or(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_v128_or(v128_t a, v128_t b) { return a | b; }

// v128_t wasm_v128_xor(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_v128_xor(v128_t a, v128_t b) { return a ^ b; }

// v128_t wasm_v128_bitselect(v128_t a, v128_t b, v128_t mask)
// `a` is selected for each lane for which `mask` is nonzero.
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_v128_bitselect(v128_t a, v128_t b, v128_t mask) {
  return (v128_t)__builtin_wasm_bitselect((__i32x4)a, (__i32x4)b, (__i32x4)mask);
}

// v128_t wasm_i8x16_neg(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_neg(v128_t a) {
  return (v128_t)(-(__u8x16)a);
}

// bool wasm_i8x16_any_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i8x16_any_true(v128_t a) {
  return __builtin_wasm_any_true_i8x16((__i8x16)a);
}

// bool wasm_i8x16_all_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i8x16_all_true(v128_t a) {
  return __builtin_wasm_all_true_i8x16((__i8x16)a);
}

// v128_t wasm_i8x16_shl(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_shl(v128_t a, int32_t b) {
  return (v128_t)((__i8x16)a << b);
}

// v128_t wasm_i8x64_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_shr(v128_t a, int32_t b) {
  return (v128_t)((__i8x16)a >> b);
}

// v128_t wasm_u8x16_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_shr(v128_t a, int32_t b) {
  return (v128_t)((__u8x16)a >> b);
}

// v128_t wasm_i8x16_add(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_add(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a + (__u8x16)b);
}

// v128_t wasm_add_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_add_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_add_saturate_s_i8x16((__i8x16)a, (__i8x16)b);
}

// v128_t wasm_add_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_add_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_add_saturate_u_i8x16((__i8x16)a, (__i8x16)b);
}

// v128_t wasm_i8x16_sub(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_sub(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a - (__u8x16)b);
}

// v128_t wasm_sub_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_sub_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_sub_saturate_s_i8x16((__i8x16)a, (__i8x16)b);
}

// v128_t wasm_sub_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u8x16_sub_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_sub_saturate_u_i8x16((__i8x16)a, (__i8x16)b);
}

// v128_t wasm_i8x16_mul(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i8x16_mul(v128_t a, v128_t b) {
  return (v128_t)((__u8x16)a * (__u8x16)b);
}

// v128_t wasm_i16x8_neg(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_neg(v128_t a) {
  return (v128_t)(-(__u16x8)a);
}

// bool wasm_i16x8_any_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i16x8_any_true(v128_t a) {
  return __builtin_wasm_any_true_i16x8((__i16x8)a);
}

// bool wasm_i16x8_all_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i16x8_all_true(v128_t a) {
  return __builtin_wasm_all_true_i16x8((__i16x8)a);
}

// v128_t wasm_i16x8_shl(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_shl(v128_t a, int32_t b) {
  return (v128_t)((__i16x8)a << b);
}

// v128_t wasm_i16x8_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_shr(v128_t a, int32_t b) {
  return (v128_t)((__i16x8)a >> b);
}

// v128_t wasm_u16x8_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_shr(v128_t a, int32_t b) {
  return (v128_t)((__u16x8)a >> b);
}

// v128_t wasm_i16x8_add(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_add(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a + (__u16x8)b);
}

// v128_t wasm_add_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_add_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_add_saturate_s_i16x8((__i16x8)a, (__i16x8)b);
}

// v128_t wasm_add_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_add_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_add_saturate_u_i16x8((__i16x8)a, (__i16x8)b);
}

// v128_t wasm_i16x8_sub(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_sub(v128_t a, v128_t b) {
  return (v128_t)((__i16x8)a - (__i16x8)b);
}

// v128_t wasm_sub_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_sub_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_sub_saturate_s_i16x8((__i16x8)a, (__i16x8)b);
}

// v128_t wasm_sub_saturate(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u16x8_sub_saturate(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_sub_saturate_u_i16x8((__i16x8)a, (__i16x8)b);
}

// v128_t wasm_i16x8_mul(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i16x8_mul(v128_t a, v128_t b) {
  return (v128_t)((__u16x8)a * (__u16x8)b);
}

// v128_t wasm_i32x4_neg(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_neg(v128_t a) {
  return (v128_t)(-(__u32x4)a);
}

// bool wasm_i32x4_any_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i32x4_any_true(v128_t a) {
  return __builtin_wasm_any_true_i32x4((__i32x4)a);
}

// bool wasm_i32x4_all_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i32x4_all_true(v128_t a) {
  return __builtin_wasm_all_true_i32x4((__i32x4)a);
}

// v128_t wasm_i32x4_shl(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_shl(v128_t a, int32_t b) {
  return (v128_t)((__i32x4)a << b);
}

// v128_t wasm_i32x4_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_shr(v128_t a, int32_t b) {
  return (v128_t)((__i32x4)a >> b);
}

// v128_t wasm_u32x4_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u32x4_shr(v128_t a, int32_t b) {
  return (v128_t)((__u32x4)a >> b);
}

// v128_t wasm_i32x4_add(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_add(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a + (__u32x4)b);
}

// v128_t wasm_i32x4_sub(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_sub(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a - (__u32x4)b);
}

// v128_t wasm_i32x4_mul(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i32x4_mul(v128_t a, v128_t b) {
  return (v128_t)((__u32x4)a * (__u32x4)b);
}

#ifdef __wasm_unimplemented_simd128__

// v128_t wasm_i64x2_neg(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_neg(v128_t a) {
  return (v128_t)(-(__u64x2)a);
}

// bool wasm_i64x2_any_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i64x2_any_true(v128_t a) {
  return __builtin_wasm_any_true_i64x2((__i64x2)a);
}

// bool wasm_i64x2_all_true(v128_t a)
static __inline__ bool __DEFAULT_FN_ATTRS wasm_i64x2_all_true(v128_t a) {
  return __builtin_wasm_all_true_i64x2((__i64x2)a);
}

// v128_t wasm_i64x2_shl(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_shl(v128_t a, int32_t b) {
  return (v128_t)((__i64x2)a << (int64_t)b);
}

// v128_t wasm_i64x2_shr(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_shr(v128_t a, int32_t b) {
  return (v128_t)((__i64x2)a >> (int64_t)b);
}

// v128_t wasm_u64x2_shr_u(v128_t a, int32_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_u64x2_shr(v128_t a, int32_t b) {
  return (v128_t)((__u64x2)a >> (int64_t)b);
}

// v128_t wasm_i64x2_add(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_add(v128_t a, v128_t b) {
  return (v128_t)((__u64x2)a + (__u64x2)b);
}

// v128_t wasm_i64x2_sub(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_i64x2_sub(v128_t a, v128_t b) {
  return (v128_t)((__u64x2)a - (__u64x2)b);
}

#endif // __wasm_unimplemented_simd128__

// v128_t  wasm_f32x4_abs(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_abs(v128_t a) {
  return (v128_t)__builtin_wasm_abs_f32x4((__f32x4)a);
}

// v128_t wasm_f32x4_neg(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_neg(v128_t a) {
  return (v128_t)(-(__f32x4)a);
}

// v128_t wasm_f32x4_sqrt(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_sqrt(v128_t a) {
  return (v128_t)__builtin_wasm_sqrt_f32x4((__f32x4)a);
}

// v128_t wasm_f32x4_qfma(v128_t a, v128_t b, v128_t c)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_qfma(v128_t a, v128_t b, v128_t c) {
  return (v128_t) __builtin_wasm_qfma_f32x4((__f32x4)a, (__f32x4)b, (__f32x4)c);
}

// v128_t wasm_f32x4_qfms(v128_t a, v128_t b, v128_t c)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_qfms(v128_t a, v128_t b, v128_t c) {
  return (v128_t) __builtin_wasm_qfms_f32x4((__f32x4)a, (__f32x4)b, (__f32x4)c);
}

// v128_t wasm_f32x4_add(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_add(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a + (__f32x4)b);
}

// v128_t wasm_f32x4_sub(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_sub(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a - (__f32x4)b);
}

// v128_t wasm_f32x4_mul(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_mul(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a * (__f32x4)b);
}

// v128_t wasm_f32x4_div(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_div(v128_t a, v128_t b) {
  return (v128_t)((__f32x4)a / (__f32x4)b);
}

// v128_t wasm_f32x4_min(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_min(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_min_f32x4((__f32x4)a, (__f32x4)b);
}

// v128_t wasm_f32x4_max(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f32x4_max(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_max_f32x4((__f32x4)a, (__f32x4)b);
}

#ifdef __wasm_unimplemented_simd128__

// v128_t  wasm_f64x2_abs(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_abs(v128_t a) {
  return (v128_t)__builtin_wasm_abs_f64x2((__f64x2)a);
}

// v128_t wasm_f64x2_neg(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_neg(v128_t a) {
  return (v128_t)(-(__f64x2)a);
}

// v128_t  wasm_f64x2_sqrt(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_sqrt(v128_t a) {
  return (v128_t)__builtin_wasm_sqrt_f64x2((__f64x2)a);
}

// v128_t wasm_f64x2_qfma(v128_t a, v128_t b, v128_t c)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_qfma(v128_t a, v128_t b, v128_t c) {
  return (v128_t) __builtin_wasm_qfma_f64x2((__f64x2)a, (__f64x2)b, (__f64x2)c);
}

// v128_t wasm_f64x2_qfms(v128_t a, v128_t b, v128_t c)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_qfms(v128_t a, v128_t b, v128_t c) {
  return (v128_t) __builtin_wasm_qfms_f64x2((__f64x2)a, (__f64x2)b, (__f64x2)c);
}

// v128_t wasm_f64x2_add(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_add(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a + (__f64x2)b);
}

// v128_t wasm_f64x2_sub(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_sub(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a - (__f64x2)b);
}

// v128_t wasm_f64x2_mul(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_mul(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a * (__f64x2)b);
}

// v128_t wasm_f64x2_div(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_div(v128_t a, v128_t b) {
  return (v128_t)((__f64x2)a / (__f64x2)b);
}

// v128_t wasm_f64x2_min(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_min(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_min_f64x2((__f64x2)a, (__f64x2)b);
}

// v128_t wasm_f64x2_max(v128_t a, v128_t b)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_f64x2_max(v128_t a, v128_t b) {
  return (v128_t)__builtin_wasm_max_f64x2((__f64x2)a, (__f64x2)b);
}

#endif // __wasm_unimplemented_simd128__

static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_trunc_saturate_i32x4_f32x4(v128_t a) {
  return (v128_t)__builtin_wasm_trunc_saturate_s_i32x4_f32x4((__f32x4)a);
}

static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_trunc_saturate_u32x4_f32x4(v128_t a) {
  return (v128_t)__builtin_wasm_trunc_saturate_u_i32x4_f32x4((__f32x4)a);
}

#ifdef __wasm_unimplemented_simd128__

static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_trunc_saturate_i64x2_f64x2(v128_t a) {
  return (v128_t)__builtin_wasm_trunc_saturate_s_i64x2_f64x2((__f64x2)a);
}

static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_trunc_saturate_u64x2_f64x2(v128_t a) {
  return (v128_t)__builtin_wasm_trunc_saturate_s_i64x2_f64x2((__f64x2)a);
}

#endif // __wasm_unimplemented_simd128__

// v128_t wasm_convert_f32x4_i32x4(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_convert_f32x4_i32x4(v128_t v) {
  return (v128_t) __builtin_convertvector((__i32x4)v, __f32x4);
}

// v128_t wasm_convert_f32x4_u32x4(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_convert_f32x4_u32x4(v128_t v) {
  return (v128_t) __builtin_convertvector((__u32x4)v, __f32x4);
}

#ifdef __wasm_unimplemented_simd128__

// v128_t wasm_convert_f64x2_i64x2(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_convert_f64x2_i64x2(v128_t v) {
  return (v128_t) __builtin_convertvector((__i64x2)v, __f64x2);
}

// v128_t wasm_convert_f64x2_u64x2(v128_t a)
static __inline__ v128_t __DEFAULT_FN_ATTRS wasm_convert_f64x2_u64x2(v128_t v) {
  return (v128_t) __builtin_convertvector((__u64x2)v, __f64x2);
}

#endif // __wasm_unimplemented_simd128__

// v128_t wasm_v8x16_shuffle(v128_t a, v128_t b, c0, ..., c15)
#define wasm_v8x16_shuffle(                                                                        \
  a, b, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15)                      \
  ((v128_t)(__builtin_shufflevector((__u8x16)(a), (__u8x16)(b), c0, c1, c2, c3, c4, c5, c6, c7,    \
    c8, c9, c10, c11, c12, c13, c14, c15)))
