#include <stdio.h>
int global = 20;
int *far;
int main() {
  int x = 5;
  int y = x + 17;
  int z = (y - 1) / 2;  // Should stay an integer after division!
  y += 1;
  int w = x * 3 + 4;
  int k = w < 15 ? 99 : 101;
  far = &k;
  *far += global;
  int i = k > 100;  // Should be an int, not a bool!
  int j = i << 6;
  j >>= 1;
  j = j ^ 5;
  int h = 1;
  h |= 0;
  int p = h;
  p &= 0;
  printf("*%d,%d,%d,%d,%d,%d,%d,%d,%d*\n", x, y, z, w, k, i, j, h, p);

  long hash = -1;
  size_t perturb;
  int ii = 0;
  for (perturb = hash;; perturb >>= 5) {
    printf("%d:%d", ii, perturb);
    ii++;
    if (ii == 9) break;
    printf(",");
  }
  printf("*\n");
  printf("*%.1d,%.2d*\n", 56, 9);

  // Fixed-point math on 64-bit ints. Tricky to support since we have no 64-bit
  // shifts in JS
  {
    struct Fixed {
      static int Mult(int a, int b) {
        return ((long long)a * (long long)b) >> 16;
      }
    };
    printf("fixed:%d\n", Fixed::Mult(150000, 140000));
  }

  printf("*%ld*%p\n", (long)21,
         &hash);  // The %p should not enter an infinite loop!
  return 0;
}
