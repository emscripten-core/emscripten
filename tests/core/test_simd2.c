#include <stdio.h>

typedef float __m128 __attribute__((__vector_size__(16)));

static inline __m128 __attribute__((always_inline))
    _mm_set_ps(const float __Z, const float __Y, const float __X,
               const float __W) {
  return (__m128) {__W, __X, __Y, __Z};
}

static inline void __attribute__((always_inline))
    _mm_store_ps(float *__P, __m128 __A) {
  *(__m128 *)__P = __A;
}

static inline __m128 __attribute__((always_inline))
    _mm_add_ps(__m128 __A, __m128 __B) {
  return __A + __B;
}

int main(int argc, char **argv) {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(9.0, 4.0, 0, -9.0);
  __m128 v2 = _mm_set_ps(7.0, 3.0, 2.5, 1.0);
  __m128 v3 = _mm_add_ps(v1, v2);
  _mm_store_ps(ar, v3);

  for (int i = 0; i < 4; i++) {
    printf("%f\n", ar[i]);
  }

  return 0;
}
