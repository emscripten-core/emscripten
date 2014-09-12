#include <time.h>
#include <errno.h>
#include <stdio.h>

int main() {
  clockid_t clocks[] = { CLOCK_REALTIME, CLOCK_MONOTONIC };
  for (int i = 0; i < (int)(sizeof(clocks)/sizeof(*clocks)); ++i) {
    printf("%sTests for clockid_t=%d\n-----------------\n",
           i == 0 ? "" : "\n", clocks[i]);
    struct timespec ts;
    int rv = clock_getres(clocks[i], &ts);
    if (rv)
      printf("clock_getres failed\n");
    else if (ts.tv_sec || ts.tv_nsec > 50000000)
      printf("clock_getres resolution not enough (%ld.%09ld)\n",
             (long)ts.tv_sec, ts.tv_nsec);
    else
      printf("clock_getres resolution OK\n");
    rv = clock_gettime(clocks[i], &ts);
    printf(rv ? "clock_gettime failed\n" : "clock_gettime OK\n");
    errno = 0;
    if (clock_settime(clocks[i], &ts) == 0)
      printf("clock_settime should have failed\n");
    else if (errno == EPERM && clocks[i] == CLOCK_REALTIME)
      printf("clock_settime failed with EPERM (OK)\n");
    else if (errno == EINVAL && clocks[i] == CLOCK_MONOTONIC)
      printf("clock_settime failed with EINVAL (OK)\n");
    else
      printf("clock_settime failed with wrong error code\n");
  }
  clockid_t bogus = 42;
  struct timespec ts;
  printf("\nTests for clockid_t=%d\n-----------------\n", bogus);
  if (clock_gettime(bogus, &ts) == 0 || errno != EINVAL)
    printf("clock_gettime should have failed\n");
  else
    printf("clock_gettime failed with EINVAL (OK)\n");
  if (clock_getres(bogus, &ts) == 0 || errno != EINVAL)
    printf("clock_getres should have failed\n");
  else
    printf("clock_getres failed with EINVAL (OK)\n");
  if (clock_settime(bogus, &ts) == 0 || errno != EINVAL)
    printf("clock_settime should have failed\n");
  else
    printf("clock_settime failed with EINVAL (OK)\n");
  return 0;
}
