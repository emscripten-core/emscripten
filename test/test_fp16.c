#include <assert.h>
#include <emscripten.h>
#include <math.h>
#include <stdio.h>
#include <wasm_simd128.h>

// Not used below, but helpful for debugging.
void print_f16x8(v128_t v1) {
  printf("[%g, %g, %g, %g, %g, %g, %g, %g]\n",
         wasm_f16x8_extract_lane(v1, 0),
         wasm_f16x8_extract_lane(v1, 1),
         wasm_f16x8_extract_lane(v1, 2),
         wasm_f16x8_extract_lane(v1, 3),
         wasm_f16x8_extract_lane(v1, 4),
         wasm_f16x8_extract_lane(v1, 5),
         wasm_f16x8_extract_lane(v1, 6),
         wasm_f16x8_extract_lane(v1, 7));
}

void assert_lanes_eq_uint16(v128_t v,
                            uint16_t l0,
                            uint16_t l1,
                            uint16_t l2,
                            uint16_t l3,
                            uint16_t l4,
                            uint16_t l5,
                            uint16_t l6,
                            uint16_t l7) {
  assert(wasm_u16x8_extract_lane(v, 0) == l0);
  assert(wasm_u16x8_extract_lane(v, 1) == l1);
  assert(wasm_u16x8_extract_lane(v, 2) == l2);
  assert(wasm_u16x8_extract_lane(v, 3) == l3);
  assert(wasm_u16x8_extract_lane(v, 4) == l4);
  assert(wasm_u16x8_extract_lane(v, 5) == l5);
  assert(wasm_u16x8_extract_lane(v, 6) == l6);
  assert(wasm_u16x8_extract_lane(v, 7) == l7);
}

void assert_lanes_eq_float(v128_t v,
                           float l0,
                           float l1,
                           float l2,
                           float l3,
                           float l4,
                           float l5,
                           float l6,
                           float l7) {
  assert(wasm_f16x8_extract_lane(v, 0) == l0);
  assert(wasm_f16x8_extract_lane(v, 1) == l1);
  assert(wasm_f16x8_extract_lane(v, 2) == l2);
  assert(wasm_f16x8_extract_lane(v, 3) == l3);
  assert(wasm_f16x8_extract_lane(v, 4) == l4);
  assert(wasm_f16x8_extract_lane(v, 5) == l5);
  assert(wasm_f16x8_extract_lane(v, 6) == l6);
  assert(wasm_f16x8_extract_lane(v, 7) == l7);
}

void assert_all_lanes_eq(v128_t v, float value) {
  assert_lanes_eq_float(
    v, value, value, value, value, value, value, value, value);
}

void assert_all_lanes_eq_uint16(v128_t v, uint16_t value) {
  assert_lanes_eq_uint16(
    v, value, value, value, value, value, value, value, value);
}

int main() {
  v128_t a, b, c;
  a = wasm_f16x8_splat(2.0f);
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_splat(1.0f);
  a = wasm_f16x8_replace_lane(a, 0, 99.0f);
  a = wasm_f16x8_replace_lane(a, 7, 99.0f);
  assert_lanes_eq_float(a, 99.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 99.0f);

  a = wasm_f16x8_abs(wasm_f16x8_splat(-2.0f));
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_neg(wasm_f16x8_splat(2.0f));
  assert_all_lanes_eq(a, -2.0f);

  a = wasm_f16x8_sqrt(wasm_f16x8_splat(4.0f));
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_ceil(wasm_f16x8_splat(1.2f));
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_floor(wasm_f16x8_splat(1.2f));
  assert_all_lanes_eq(a, 1.0f);

  a = wasm_f16x8_trunc(wasm_f16x8_splat(1.2f));
  assert_all_lanes_eq(a, 1.0f);

  a = wasm_f16x8_nearest(wasm_f16x8_splat(1.5f));
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_eq(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq_uint16(a, 0);

  a = wasm_f16x8_ne(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq_uint16(a, 0xFFFF);

  a = wasm_f16x8_lt(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq_uint16(a, 0xFFFF);

  a = wasm_f16x8_gt(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq_uint16(a, 0);

  a = wasm_f16x8_le(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq_uint16(a, 0xFFFF);

  a = wasm_f16x8_ge(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq_uint16(a, 0);

  a = wasm_f16x8_add(wasm_f16x8_splat(2.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq(a, 5.0f);

  a = wasm_f16x8_sub(wasm_f16x8_splat(5.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_mul(wasm_f16x8_splat(5.0f), wasm_f16x8_splat(3.0f));
  assert_all_lanes_eq(a, 15.0f);

  a = wasm_f16x8_div(wasm_f16x8_splat(10.0f), wasm_f16x8_splat(5.0f));
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_min(wasm_f16x8_splat(10.0f), wasm_f16x8_splat(5.0f));
  assert_all_lanes_eq(a, 5.0f);

  a = wasm_f16x8_max(wasm_f16x8_splat(10.0f), wasm_f16x8_splat(5.0f));
  assert_all_lanes_eq(a, 10.0f);

  a = wasm_f16x8_pmin(wasm_f16x8_splat(10.0f), wasm_f16x8_splat(5.0f));
  assert_all_lanes_eq(a, 5.0f);

  a = wasm_f16x8_pmax(wasm_f16x8_splat(10.0f), wasm_f16x8_splat(5.0f));
  assert_all_lanes_eq(a, 10.0f);

  // TODO needed in binaryen
  // wasm_i16x8_trunc_sat_f16x8;
  // wasm_u16x8_trunc_sat_f16x8
  // wasm_f16x8_convert_i16x8
  // wasm_f16x8_convert_u16x8

  a = wasm_f16x8_relaxed_madd(
    wasm_f16x8_splat(2), wasm_f16x8_splat(3), wasm_f16x8_splat(4));
  assert_all_lanes_eq(a, 10.0f);

  a = wasm_f16x8_relaxed_nmadd(
    wasm_f16x8_splat(2), wasm_f16x8_splat(3), wasm_f16x8_splat(4));
  assert_all_lanes_eq(a, -2.0f);
}
