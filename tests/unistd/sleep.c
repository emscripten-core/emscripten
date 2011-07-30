#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int main() {
  time_t start;
  time_t taken;

  start = time(0);
  printf("sleep(2) ret: %d\n", sleep(2));
  taken = time(0) - start;
  printf("after sleep(2) between 2 and 3: %d\n", taken >= 2 && taken <= 3);
  printf("errno: %d\n", errno);
  errno = 0;

  start = time(0);
  printf("usleep(3000000) ret: %d\n", usleep(3000000));
  taken = time(0) - start;
  printf("after usleep(3000000) between 3 and 4: %d\n", taken >= 3 && taken <= 4);
  printf("errno: %d\n", errno);

  return 0;
}
