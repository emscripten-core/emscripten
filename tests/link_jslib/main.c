#include <stdio.h>

extern "C" {
  extern int test_link_jslib(int a, int b);
}

int main() {
  int temp = test_link_jslib(11, 31);
  printf("res: %d\n", temp);
  return 0;
}
