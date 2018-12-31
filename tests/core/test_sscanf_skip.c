#include <stdint.h>
#include <stdio.h>

int main() {
  int val1;
  printf("%d\n", sscanf("10 20 30 40", "%*lld %*d %d", &val1));
  printf("%d\n", val1);

  int64_t large, val2;
  printf("%d\n", sscanf("1000000 -1125899906842620 -123 -1073741823",
                        "%lld %*lld %lld %*d", &large, &val2));
  printf("%lld,%lld\n", large, val2);

  return 0;
}
