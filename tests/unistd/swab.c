#include <stdio.h>
#include <unistd.h>

int main() {
  char src[] = "abcdefg";
  char dst[10] = {0};
  swab(src, dst, 5);

  printf("before: %s\n", src);
  printf("after: %s\n", dst);

  return 0;
}
