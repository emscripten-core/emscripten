#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>

/*
 * x86 _mm_max_ps and _mm_min_ps have defined semantics or NaN and -0.0. Ensure
 * that we implement them properly.
 */

volatile __m128 negzero;
volatile __m128 zero;
volatile __m128 nan_;
volatile __m128 one;

int main() {
  negzero = _mm_set1_ps(-0.0);
  zero = _mm_set1_ps(0.0);
  one = _mm_set1_ps(1.0);
  nan_ = _mm_set1_ps(__builtin_nan(""));

  __m128 expected, result;

  result = _mm_max_ps(negzero, zero);
  expected = zero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_max_ps(zero, negzero);
  expected = negzero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_max_ps(nan_, zero);
  expected = zero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_max_ps(zero, nan_);
  expected = nan_;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_max_ps(zero, one);
  expected = one;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_min_ps(negzero, zero);
  expected = zero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_min_ps(zero, negzero);
  expected = negzero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_min_ps(nan_, zero);
  expected = zero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_min_ps(zero, nan_);
  expected = nan_;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  result = _mm_min_ps(zero, one);
  expected = zero;
  assert(memcmp(&result, &expected, sizeof(__m128)) == 0);

  printf("DONE");
  return 0;
}
