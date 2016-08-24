#include <stdio.h>
struct bitty {
  unsigned x : 1;
  unsigned y : 1;
  unsigned z : 1;
};
int main() {
  bitty b;
  printf("*");
  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++)
      for (int k = 0; k <= 1; k++) {
        b.x = i;
        b.y = j;
        b.z = k;
        printf("%d,%d,%d,", b.x, b.y, b.z);
      }
  printf("*\n");
  return 0;
}
