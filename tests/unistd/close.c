#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int f = open("/", O_RDONLY);

  printf("fsync(opened): %d\n", fsync(f));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("close(opened): %d\n", close(f));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("fsync(closed): %d\n", fsync(f));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("close(closed): %d\n", close(f));
  printf("errno: %d\n", errno);
  errno = 0;

  return 0;
}
