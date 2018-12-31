#include <stdio.h>
#include <sys/time.h>

typedef long long int64;

#define PRMJ_USEC_PER_SEC 1000000L

int main(int argc, char* argv[]) {
  int64 sec = 1329409675 + argc;
  int64 usec = 2329509675;
  int64 mul = int64(sec) * PRMJ_USEC_PER_SEC;
  int64 add = mul + int64(usec);
  int add_low = add;
  int add_high = add >> 32;
  printf("*%lld,%lld,%u,%u*\n", mul, add, add_low, add_high);
  int64 x = sec + (usec << 25);
  x >>= argc * 3;
  printf("*%llu*\n", x);
  return 0;
}
