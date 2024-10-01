#include <assert.h>
#include <emscripten.h>
#include <math.h>
#include <stdio.h>
#include <wasm_simd128.h>

// SIMD True/False lane values.
#define TRUE 0xFFFF
#define FALSE 0

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
  // We can't do a direct comparision since there may be quiet or signaling nan
  // values, so if the expected value is nan check that the actual value is nan
  // too.
  float actual;
  actual = wasm_f16x8_extract_lane(v, 0);
  assert(isnan(l0) ? isnan(actual) : actual == l0);
  actual = wasm_f16x8_extract_lane(v, 1);
  assert(isnan(l1) ? isnan(actual) : actual == l1);
  actual = wasm_f16x8_extract_lane(v, 2);
  assert(isnan(l2) ? isnan(actual) : actual == l2);
  actual = wasm_f16x8_extract_lane(v, 3);
  assert(isnan(l3) ? isnan(actual) : actual == l3);
  actual = wasm_f16x8_extract_lane(v, 4);
  assert(isnan(l4) ? isnan(actual) : actual == l4);
  actual = wasm_f16x8_extract_lane(v, 5);
  assert(isnan(l5) ? isnan(actual) : actual == l5);
  actual = wasm_f16x8_extract_lane(v, 6);
  assert(isnan(l6) ? isnan(actual) : actual == l6);
  actual = wasm_f16x8_extract_lane(v, 7);
  assert(isnan(l7) ? isnan(actual) : actual == l7);
}

void assert_all_lanes_eq(v128_t v, float value) {
  assert_lanes_eq_float(
    v, value, value, value, value, value, value, value, value);
}

v128_t create_f16x8(float l0,
                    float l1,
                    float l2,
                    float l3,
                    float l4,
                    float l5,
                    float l6,
                    float l7) {
  v128_t v;
  v = wasm_f16x8_replace_lane(v, 0, l0);
  v = wasm_f16x8_replace_lane(v, 1, l1);
  v = wasm_f16x8_replace_lane(v, 2, l2);
  v = wasm_f16x8_replace_lane(v, 3, l3);
  v = wasm_f16x8_replace_lane(v, 4, l4);
  v = wasm_f16x8_replace_lane(v, 5, l5);
  v = wasm_f16x8_replace_lane(v, 6, l6);
  v = wasm_f16x8_replace_lane(v, 7, l7);
  return v;
}

int main() {
  v128_t a, b, c;
  a = wasm_f16x8_splat(2.0f);
  assert_all_lanes_eq(a, 2.0f);

  a = wasm_f16x8_splat(1.0f);
  a = wasm_f16x8_replace_lane(a, 0, 99.0f);
  a = wasm_f16x8_replace_lane(a, 7, 99.0f);
  assert_lanes_eq_float(a, 99.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 99.0f);

  // The following tests are not exhaustive, but try to hit some interesting
  // floats and how they interact.

  a = wasm_f16x8_abs(create_f16x8(-1.0f, 1.0f, 0.0f, -0.0f, INFINITY, -INFINITY, NAN, -NAN));
  assert_lanes_eq_float(a,         1.0f, 1.0f, 0.0f,  0.0f, INFINITY,  INFINITY, NAN,  NAN);

  a = wasm_f16x8_neg(create_f16x8(-1.0f, 1.0f,   0.0f, -0.0f,  INFINITY, -INFINITY, NAN, -NAN));
  assert_lanes_eq_float(a,         1.0f, -1.0f, -0.0f,  0.0f, -INFINITY,  INFINITY, NAN,  NAN);

  a = wasm_f16x8_sqrt(create_f16x8(-1.0f, 1.0f,  0.0f,  4.0f, INFINITY, -INFINITY, NAN, -NAN));
  assert_lanes_eq_float(a,           NAN, 1.0f,  0.0f,  2.0f, INFINITY,       NAN, NAN,  NAN);

  a = wasm_f16x8_ceil(create_f16x8(-1.0f, 1.0f,  -1.2f,  1.2f, -1.7f, 1.7f, INFINITY, -INFINITY));
  assert_lanes_eq_float(a,         -1.0f, 1.0f,  -1.0f,  2.0f, -1.0f, 2.0f, INFINITY, -INFINITY);

  a = wasm_f16x8_floor(create_f16x8(-1.0f, 1.0f,  -1.2f,  1.2f, -1.7f, 1.7f, INFINITY, -INFINITY));
  assert_lanes_eq_float(a,          -1.0f, 1.0f,  -2.0f,  1.0f, -2.0f, 1.0f, INFINITY, -INFINITY);

  a = wasm_f16x8_trunc(create_f16x8(-1.0f, 1.0f,  -1.2f,  1.2f, -1.7f, 1.7f, INFINITY, -INFINITY));
  assert_lanes_eq_float(a,          -1.0f, 1.0f,  -1.0f,  1.0f, -1.0f, 1.0f, INFINITY, -INFINITY);

  a = wasm_f16x8_nearest(create_f16x8(-1.0f, 1.0f,  -1.2f,  1.2f, -1.7f, 1.7f, INFINITY, -INFINITY));
  assert_lanes_eq_float(a,            -1.0f, 1.0f,  -1.0f,  1.0f, -2.0f, 2.0f, INFINITY, -INFINITY);

  a = wasm_f16x8_eq(create_f16x8(-1.0f,  1.0f, 1.0f, -0.0f, INFINITY, -INFINITY,  NAN,  -NAN),
                    create_f16x8( 1.0f, -1.0f, 1.0f, -0.0f, INFINITY, -INFINITY,  NAN,  -NAN));
  assert_lanes_eq_uint16(a,      FALSE, FALSE, TRUE,  TRUE,     TRUE,      TRUE, FALSE, FALSE);

  a = wasm_f16x8_ne(create_f16x8(-1.0f,  1.0f, 1.0f,  -0.0f, INFINITY, -INFINITY,   NAN, -NAN),
                    create_f16x8( 1.0f, -1.0f, 1.0f,  -0.0f, INFINITY, -INFINITY,   NAN, -NAN));
  assert_lanes_eq_uint16(a,       TRUE,  TRUE, FALSE, FALSE,    FALSE,     FALSE,  TRUE, TRUE);

  a = wasm_f16x8_lt(create_f16x8(-1.0f,  1.0f,  1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN),
                    create_f16x8( 1.0f, -1.0f,  1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN));
  assert_lanes_eq_uint16(a,       TRUE, FALSE, FALSE, FALSE,     FALSE,    FALSE, FALSE, FALSE);

  a = wasm_f16x8_gt(create_f16x8( -1.0f,  1.0f,  1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN),
                    create_f16x8(  1.0f, -1.0f,  1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN));
  assert_lanes_eq_uint16(a,       FALSE,  TRUE, FALSE, FALSE,     FALSE,    FALSE, FALSE, FALSE);

  a = wasm_f16x8_le(create_f16x8(-1.0f,  1.0f, 1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN),
                    create_f16x8( 1.0f, -1.0f, 1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN));
  assert_lanes_eq_uint16(a,       TRUE, FALSE, TRUE,  TRUE,     TRUE,      TRUE, FALSE, FALSE);

  a = wasm_f16x8_ge(create_f16x8(-1.0f,  1.0f, 1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN),
                    create_f16x8( 1.0f, -1.0f, 1.0f, -0.0f, INFINITY, -INFINITY,   NAN,  -NAN));
  assert_lanes_eq_uint16(a,       FALSE, TRUE, TRUE,  TRUE,     TRUE,      TRUE, FALSE, FALSE);

  a = wasm_f16x8_add(create_f16x8(-1.0f,  1.0f, 1.0f,  1.0f, INFINITY, INFINITY,  NAN, NAN),
                     create_f16x8( 1.0f, -1.0f, 1.0f,  0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,         0.0f,  0.0f, 2.0f,  1.0f, INFINITY, INFINITY,  NAN, NAN);

  a = wasm_f16x8_sub(create_f16x8(-1.0f,  1.0f, 1.0f,  1.0f, INFINITY, INFINITY,  NAN, NAN),
                     create_f16x8( 1.0f, -1.0f, 1.0f,  0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,        -2.0f,  2.0f, 0.0f,  1.0f, INFINITY,      NAN,  NAN, NAN);

  a = wasm_f16x8_mul(create_f16x8(-1.0f,  1.0f, 1.0f,  1.0f, INFINITY, INFINITY,  NAN, NAN),
                     create_f16x8( 1.0f, -1.0f, 1.0f,  0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,        -1.0f, -1.0f, 1.0f,  0.0f, INFINITY, INFINITY,  NAN, NAN);

  a = wasm_f16x8_div(create_f16x8(-1.0f,  1.0f, 1.0f,      1.0f, INFINITY, INFINITY,  NAN, NAN),
                     create_f16x8( 1.0f, -1.0f, 1.0f,      0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,        -1.0f, -1.0f, 1.0f,  INFINITY, INFINITY,      NAN,  NAN, NAN);

  a = wasm_f16x8_min(create_f16x8(-1.0f,  1.0f, 1.0f, 1.0f, INFINITY, INFINITY,  NAN, NAN),
                     create_f16x8( 1.0f, -1.0f, 1.0f, 0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,        -1.0f, -1.0f, 1.0f, 0.0f,     1.0f, INFINITY,  NAN, NAN);

  a = wasm_f16x8_max(create_f16x8(-1.0f,  1.0f, 1.0f, 1.0f, INFINITY, INFINITY,  NAN, NAN),
                     create_f16x8( 1.0f, -1.0f, 1.0f, 0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,         1.0f,  1.0f, 1.0f, 1.0f, INFINITY, INFINITY,  NAN, NAN);

  a = wasm_f16x8_pmin(create_f16x8(-1.0f,  1.0f, 1.0f, 1.0f, INFINITY, INFINITY,  NAN, NAN),
                      create_f16x8( 1.0f, -1.0f, 1.0f, 0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,         -1.0f, -1.0f, 1.0f, 0.0f,     1.0f, INFINITY,  NAN, NAN);

  a = wasm_f16x8_pmax(create_f16x8(-1.0f,  1.0f, 1.0f, 1.0f, INFINITY, INFINITY,  NAN, NAN),
                      create_f16x8( 1.0f, -1.0f, 1.0f, 0.0f,     1.0f, INFINITY, 1.0f, NAN));
  assert_lanes_eq_float(a,          1.0f,  1.0f, 1.0f, 1.0f, INFINITY, INFINITY,  NAN, NAN);

  // TODO needed in binaryen
  // wasm_i16x8_trunc_sat_f16x8;
  // wasm_u16x8_trunc_sat_f16x8
  // wasm_f16x8_convert_i16x8
  // wasm_f16x8_convert_u16x8

  // Lane 0 illustrates the difference between fused/unfused. e.g.
  // fused: (positive overflow) + -inf = -inf
  // unfused: (inf) + -inf = NaN
  a = wasm_f16x8_relaxed_madd(create_f16x8(      1e4, INFINITY, -1.0f, 0.0f, 1.0f,  1.5f, -2.0f,  1.0f),
                              create_f16x8(      1e4, INFINITY, -1.0f, 0.0f, 1.0f,  1.5f,  4.0f,  1.0f),
                              create_f16x8(-INFINITY, INFINITY, -1.0f, 0.0f, 1.0f,  2.0f,  1.0f, -1.0f));
  assert_lanes_eq_float(a,                 -INFINITY, INFINITY,  0.0f, 0.0f, 2.0f, 4.25f, -7.0f,  0.0f);

  // Lane 0 illustrates the difference between fused/unfused. e.g.
  // fused: -(positive overflow) + inf = inf
  // unfused: (-inf) + -inf = NaN
  a = wasm_f16x8_relaxed_nmadd(create_f16x8(     1e4, -INFINITY, -1.0f, 0.0f, 1.0f,   1.5f, -2.0f,  1.0f),
                               create_f16x8(     1e4,  INFINITY, -1.0f, 0.0f, 1.0f,   1.5f,  4.0f,  1.0f),
                               create_f16x8(INFINITY,  INFINITY, -1.0f, 0.0f, 1.0f,   2.0f,  1.0f, -1.0f));
  assert_lanes_eq_float(a,                  INFINITY,  INFINITY, -2.0f, 0.0f, 0.0f, -0.25f,  9.0f, -2.0f);
}
