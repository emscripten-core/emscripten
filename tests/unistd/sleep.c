#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int main() {
  time_t start;

  start = time(0);
  printf("sleep(2) ret: %d\n", sleep(2));
  printf("after sleep(2): %d\n", time(0) - start);
  printf("errno: %d\n", errno);
  errno = 0;

  start = time(0);
  printf("usleep(3000000) ret: %d\n", usleep(3100000));
  printf("after usleep(3000000): %d\n", time(0) - start);
  printf("errno: %d\n", errno);

  return 0;
}
