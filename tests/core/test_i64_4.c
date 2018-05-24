#include <stdio.h>
#include <stdint.h>

int main()
{
  // i32 vs i64
  int32_t small = -1;
  int64_t large = -1;
  printf("*%d*\n", small == large);
  small++;
  printf("*%d*\n", small == large);
  uint32_t usmall = -1;
  uint64_t ularge = -1;
  printf("*%d*\n", usmall == ularge);
  return 0;
}
