#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef unsigned int uint32;

int cmp_uint(const void *i1, const void *i2) {
  if (*static_cast<const uint32*>(i1) >
      *static_cast<const uint32*>(i2))
    return 1;

  if (*static_cast<const uint32*>(i1) <
      *static_cast<const uint32*>(i2))
    return -1;

  return 0;
}

int main() {
  clock_t start = clock();
  const size_t TIMES = 10000;
  for (size_t i = 0; i < TIMES; i++) {
    const size_t num = 100;
    uint32 rnd[num] = { \
      407, 236, 765, 529,  24,  13, 577, 900, 242, 245, \
      782, 972, 514, 100, 596, 470, 680,  65, 370, 788, \
       44, 330, 579, 314, 914, 399, 100, 945, 992, 412, \
      308, 102, 895, 529, 216, 422, 851, 778,  28, 804, \
      325, 975, 961, 623, 922, 667, 141, 755, 416, 575, \
      712, 503, 174, 675,  14, 647, 544, 881, 858, 621, \
       26, 283, 460, 252, 146,  16, 571, 570,  14, 143, \
      674, 985, 477, 386, 932, 490, 611, 127, 702, 619, \
      104, 892,  58, 635, 663, 424, 714, 740, 229, 538, \
      167, 181, 193, 193, 657, 778, 217, 573, 764, 745};

    qsort(rnd, num, sizeof(uint32), cmp_uint);
  }
  clock_t end = clock();

  float diff = (((float)end - (float)start) / CLOCKS_PER_SEC ) * 1000;
  printf("cost %fms\n", diff);
}

