#include <stdio.h>

typedef float float32x4 __attribute__((__vector_size__(16)));

int main(int argc, char **argv) {
  float32x4 a = {1.0, 2.0, 3.0, 4.0};
  float32x4 b = {5.0, 6.0, 7.0, 8.0};
  float32x4 r = __builtin_shufflevector(a, b, 3, 1, 2, 0);
  printf("%.1f %.1f %.1f %.1f\n", r[0], r[1], r[2], r[3]);
  r = __builtin_shufflevector(a, b, 7, 5, 6, 4);
  printf("%.1f %.1f %.1f %.1f\n", r[0], r[1], r[2], r[3]);
  r = __builtin_shufflevector(a, b, 1, 2, 5, 4);
  printf("%.1f %.1f %.1f %.1f\n", r[0], r[1], r[2], r[3]);
  r = __builtin_shufflevector(a, b, 5, 4, 1, 2);
  printf("%.1f %.1f %.1f %.1f\n", r[0], r[1], r[2], r[3]);
  r = __builtin_shufflevector(a, b, 4, 1, 0, 5);
  printf("%.1f %.1f %.1f %.1f\n", r[0], r[1], r[2], r[3]);
  r = __builtin_shufflevector(a, b, 0, 7, 2, 3);
  printf("%.1f %.1f %.1f %.1f\n", r[0], r[1], r[2], r[3]);
  return 0;
}