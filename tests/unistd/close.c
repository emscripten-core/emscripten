#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

int main() {
  int f = open("/", O_RDONLY);

  int ret = fsync(f);
  printf("fsync(opened): %d\n", ret);
  printf("errno: %d\n", errno);
  assert(ret == 0);
  assert(errno == 0);
  errno = 0;

  ret = close(f);
  printf("close(opened): %d\n", ret);
  printf("errno: %d\n", errno);
  assert(ret == 0);
  assert(errno == 0);
  errno = 0;

  ret = fsync(f);
  printf("fsync(closed): %d\n", ret);
  printf("errno: %d\n", errno);
  assert(ret == -1);
  assert(errno == EBADF);
  errno = 0;

  ret = close(f);
  printf("close(closed): %d\n", ret);
  printf("errno: %d\n", errno);
  assert(ret == -1);
  assert(errno == EBADF);
  errno = 0;

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  return 0;
}
