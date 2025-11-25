#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

int main() {
  struct rusage u;
  getrusage(RUSAGE_SELF, &u);
  printf("ru_utime.tv_sec: %lld\n", u.ru_utime.tv_sec);
  printf("ru_utime.tv_usec: %d\n", u.ru_utime.tv_usec);
  printf("ru_stime.tv_sec: %lld\n", u.ru_stime.tv_sec);
  printf("ru_stime.tv_usec: %d\n", u.ru_stime.tv_usec);
  return 0;
}
