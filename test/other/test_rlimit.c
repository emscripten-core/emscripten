#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/resource.h>

int main() {
  struct rlimit rlim;

  assert(getrlimit(RLIMIT_NOFILE, &rlim) == 0);
  printf("RLIMIT_NOFILE: rlim_cur: %lld\n", rlim.rlim_cur);
  printf("RLIMIT_NOFILE: rlim_max: %lld\n", rlim.rlim_max);

  assert(getrlimit(RLIMIT_CORE, &rlim) == 0);
  printf("RLIMIT_CORE: rlim_cur: %lld\n", rlim.rlim_cur);
  printf("RLIMIT_CORE: rlim_max: %lld\n", rlim.rlim_max);

  // setrlimit should always fail with EPERM
  assert(setrlimit(RLIMIT_NOFILE, &rlim) == -1);
  assert(errno == EPERM);

  return 0;
}
