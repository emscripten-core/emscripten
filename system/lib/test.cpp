#include <stdio.h>
#include <time.h>

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  printf("BAD\n");
  return 0;
}

/*
int main() {
  printf("ok\n");
}
*/

