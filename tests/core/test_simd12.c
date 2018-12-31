#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>

volatile __m128 zero;
volatile __m128 one;
volatile __m128 nan_;

int main() {
  zero = _mm_set1_ps(0);
  one = _mm_set1_ps(1);
  nan_ = _mm_set1_ps(__builtin_nan(""));

  assert(_mm_comieq_ss(zero, zero) == 1);
  assert(_mm_comieq_ss(zero, one) == 0);
  assert(_mm_comieq_ss(one, zero) == 0);
  assert(_mm_comieq_ss(zero, nan_) == 1);
  assert(_mm_comieq_ss(nan_, zero) == 1);
  assert(_mm_comieq_ss(nan_, nan_) == 1);

  assert(_mm_comige_ss(zero, zero) == 1);
  assert(_mm_comige_ss(zero, one) == 0);
  assert(_mm_comige_ss(one, zero) == 1);
  assert(_mm_comige_ss(zero, nan_) == 0);
  assert(_mm_comige_ss(nan_, zero) == 0);
  assert(_mm_comige_ss(nan_, nan_) == 0);

  assert(_mm_comigt_ss(zero, zero) == 0);
  assert(_mm_comigt_ss(zero, one) == 0);
  assert(_mm_comigt_ss(one, zero) == 1);
  assert(_mm_comigt_ss(zero, nan_) == 0);
  assert(_mm_comigt_ss(nan_, zero) == 0);
  assert(_mm_comigt_ss(nan_, nan_) == 0);

  assert(_mm_comile_ss(zero, zero) == 1);
  assert(_mm_comile_ss(zero, one) == 1);
  assert(_mm_comile_ss(one, zero) == 0);
  assert(_mm_comile_ss(zero, nan_) == 1);
  assert(_mm_comile_ss(nan_, zero) == 1);
  assert(_mm_comile_ss(nan_, nan_) == 1);

  assert(_mm_comilt_ss(zero, zero) == 0);
  assert(_mm_comilt_ss(zero, one) == 1);
  assert(_mm_comilt_ss(one, zero) == 0);
  assert(_mm_comilt_ss(zero, nan_) == 1);
  assert(_mm_comilt_ss(nan_, zero) == 1);
  assert(_mm_comilt_ss(nan_, nan_) == 1);

  assert(_mm_comineq_ss(zero, zero) == 0);
  assert(_mm_comineq_ss(zero, one) == 1);
  assert(_mm_comineq_ss(one, zero) == 1);
  assert(_mm_comineq_ss(zero, nan_) == 0);
  assert(_mm_comineq_ss(nan_, zero) == 0);
  assert(_mm_comineq_ss(nan_, nan_) == 0);

  assert(_mm_ucomieq_ss(zero, zero) == 1);
  assert(_mm_ucomieq_ss(zero, one) == 0);
  assert(_mm_ucomieq_ss(one, zero) == 0);
  assert(_mm_ucomieq_ss(zero, nan_) == 1);
  assert(_mm_ucomieq_ss(nan_, zero) == 1);
  assert(_mm_ucomieq_ss(nan_, nan_) == 1);

  assert(_mm_ucomige_ss(zero, zero) == 1);
  assert(_mm_ucomige_ss(zero, one) == 0);
  assert(_mm_ucomige_ss(one, zero) == 1);
  assert(_mm_ucomige_ss(zero, nan_) == 0);
  assert(_mm_ucomige_ss(nan_, zero) == 0);
  assert(_mm_ucomige_ss(nan_, nan_) == 0);

  assert(_mm_ucomigt_ss(zero, zero) == 0);
  assert(_mm_ucomigt_ss(zero, one) == 0);
  assert(_mm_ucomigt_ss(one, zero) == 1);
  assert(_mm_ucomigt_ss(zero, nan_) == 0);
  assert(_mm_ucomigt_ss(nan_, zero) == 0);
  assert(_mm_ucomigt_ss(nan_, nan_) == 0);

  assert(_mm_ucomile_ss(zero, zero) == 1);
  assert(_mm_ucomile_ss(zero, one) == 1);
  assert(_mm_ucomile_ss(one, zero) == 0);
  assert(_mm_ucomile_ss(zero, nan_) == 1);
  assert(_mm_ucomile_ss(nan_, zero) == 1);
  assert(_mm_ucomile_ss(nan_, nan_) == 1);

  assert(_mm_ucomilt_ss(zero, zero) == 0);
  assert(_mm_ucomilt_ss(zero, one) == 1);
  assert(_mm_ucomilt_ss(one, zero) == 0);
  assert(_mm_ucomilt_ss(zero, nan_) == 1);
  assert(_mm_ucomilt_ss(nan_, zero) == 1);
  assert(_mm_ucomilt_ss(nan_, nan_) == 1);

  assert(_mm_ucomineq_ss(zero, zero) == 0);
  assert(_mm_ucomineq_ss(zero, one) == 1);
  assert(_mm_ucomineq_ss(one, zero) == 1);
  assert(_mm_ucomineq_ss(zero, nan_) == 0);
  assert(_mm_ucomineq_ss(nan_, zero) == 0);
  assert(_mm_ucomineq_ss(nan_, nan_) == 0);

  printf("DONE!\n");
}
