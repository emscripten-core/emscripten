#include <stdio.h>

static const struct {
  unsigned char left;
  unsigned char right;
} prioritah[] = {{6, 6}, {6, 6}, {7, 95}, {7, 7}};

int main() {
  printf("*%d,%d\n", prioritah[1].left, prioritah[1].right);
  printf("%d,%d*\n", prioritah[2].left, prioritah[2].right);
  return 0;
}
