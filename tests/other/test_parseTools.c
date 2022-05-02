#include <limits.h>
#include <stdio.h>
#include <inttypes.h>

void test_makeGetValue(int32_t* ptr);

int main() {
  int32_t num = -0x12345678;
  test_makeGetValue(&num);

  printf("done\n");
  return 0;
}
