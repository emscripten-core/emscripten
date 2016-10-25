#include <stdio.h>

#include <emscripten/vector.h>

static inline float32x4 __attribute__((always_inline))
    _mm_set_ps(const float __Z, const float __Y, const float __X,
               const float __W) {
  return (float32x4) {__W, __X, __Y, __Z};
}

static __inline__ float32x4 __attribute__((__always_inline__))
    _mm_setzero_ps(void) {
  return (float32x4) {0.0, 0.0, 0.0, 0.0};
}

int main(int argc, char **argv) {
  float data[8];
  for (int i = 0; i < 32; i++)
    data[i] = (1 + i + argc) * (2 + i + argc * argc);  // confuse optimizer
  {
    float32x4 *a = (float32x4 *)&data[0];
    float32x4 *b = (float32x4 *)&data[4];
    float32x4 c, d;
    c = *a;
    d = *b;
    printf("1floats! %d, %d, %d, %d   %d, %d, %d, %d\n", (int)c[0], (int)c[1],
           (int)c[2], (int)c[3], (int)d[0], (int)d[1], (int)d[2], (int)d[3]);
    c = c + d;
    printf("2floats! %d, %d, %d, %d   %d, %d, %d, %d\n", (int)c[0], (int)c[1],
           (int)c[2], (int)c[3], (int)d[0], (int)d[1], (int)d[2], (int)d[3]);
    d = c * d;
    printf("3floats! %d, %d, %d, %d   %d, %d, %d, %d\n", (int)c[0], (int)c[1],
           (int)c[2], (int)c[3], (int)d[0], (int)d[1], (int)d[2], (int)d[3]);
    c = _mm_setzero_ps();
    printf("zeros %d, %d, %d, %d\n", (int)c[0], (int)c[1], (int)c[2],
           (int)c[3]);
  }
  {
    int32x4 *a = (int32x4 *)&data[0];
    int32x4 *b = (int32x4 *)&data[4];
    int32x4 c, d, e, f;
    c = *a;
    d = *b;
    printf("4ints! %d, %d, %d, %d   %d, %d, %d, %d\n", c[0], c[1], c[2], c[3],
           d[0], d[1], d[2], d[3]);
    e = c + d;
    f = c - d;
    printf("5ints! %d, %d, %d, %d   %d, %d, %d, %d\n", e[0], e[1], e[2], e[3],
           f[0], f[1], f[2], f[3]);
    e = c & d;
    f = c | d;
    e = ~c & d;
    f = c ^ d;
    printf("5intops! %d, %d, %d, %d   %d, %d, %d, %d\n", e[0], e[1], e[2], e[3],
           f[0], f[1], f[2], f[3]);
  }
  {
    float32x4 c, d, e, f;
    c = _mm_set_ps(9.0, 4.0, 0, -9.0);
    d = _mm_set_ps(10.0, 14.0, -12, -2.0);
    printf("6floats! %d, %d, %d, %d   %d, %d, %d, %d\n", (int)c[0], (int)c[1],
           (int)c[2], (int)c[3], (int)d[0], (int)d[1], (int)d[2], (int)d[3]);
  }

  return 0;
}
