#include <stdio.h>
struct Random {
  enum {
    IM = 139968,
    IA = 3877,
    IC = 29573
  };
  Random() : last(42) {}
  float get(float max = 1.0f) {
    last = (last * IA + IC) % IM;
    return max * last / IM;
  }

 protected:
  unsigned int last;
} rng1;
int main() {
  Random rng2;
  int count = 0;
  for (int i = 0; i < 100; i++) {
    float x1 = rng1.get();
    float x2 = rng2.get();
    printf("%f, %f\n", x1, x2);
    if (x1 != x2) count += 1;
  }
  printf("*%d*\n", count);
  return 0;
}
