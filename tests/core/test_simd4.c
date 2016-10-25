#include <stdio.h>
#include <xmmintrin.h>

float simdAverage(float *src, int len) {
  __m128 sumx4 = _mm_setzero_ps();
  for (int i = 0; i < len; i += 4) {
    __m128 v = _mm_load_ps(src);
    sumx4 = _mm_add_ps(sumx4, v);
    src += 4;
  }
  float sumx4_mem[4];
  float *sumx4_ptr = sumx4_mem;
  _mm_store_ps(sumx4_ptr, sumx4);
  return (sumx4_mem[0] + sumx4_mem[1] +
          sumx4_mem[2] + sumx4_mem[3])/len;
}

void initArray(float *src, int len) {
	for (int i = 0; i < len; ++i) {
    src[i] = 0.1 * i;
  }
}

int main() {
  const int len = 100000;
  float src[len];
  float result = 0.0;

  initArray(src, len);

  result = simdAverage(src, len);
  printf("averagex4 result: %.1f\n", result);
}