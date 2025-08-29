#include <stdio.h>
#include <xmmintrin.h>

// Calculate length of a 4D float vector using SSE1
float vec4_length_sse(const float v[4]) {
  __m128 x = _mm_loadu_ps(v);          // load vector
  __m128 sq = _mm_mul_ps(x, x);        // square each element
  __m128 shuf = _mm_shuffle_ps(sq, sq, _MM_SHUFFLE(2, 3, 0, 1)); // swap pairs
  __m128 sums = _mm_add_ps(sq, shuf);  // add pairwise
  shuf = _mm_movehl_ps(shuf, sums);    // high half -> low
  sums = _mm_add_ss(sums, shuf);       // add final two
  return _mm_cvtss_f32(_mm_sqrt_ss(sums)); // sqrt of sum of squares
}

int main() {
  float v[4] = { 3, 4, 0, 0 };
  printf("Length of a triangle with sides 3 and 4 is %f.\n", vec4_length_sse(v));
}
