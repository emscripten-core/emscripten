#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main() {
  struct stat s;
  int f = open("/test", O_RDWR, 0777);
  assert(f);

  printf("posix_fadvise: %d\n", posix_fadvise(f, 3, 2, POSIX_FADV_DONTNEED));
  printf("errno: %d\n", errno);
  printf("\n");
  errno = 0;

  printf("posix_fallocate: %d\n", posix_fallocate(f, 3, 2));
  printf("errno: %d\n", errno);
  stat("/test", &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  printf("\n");
  errno = 0;

  printf("posix_fallocate2: %d\n", posix_fallocate(f, 3, 7));
  printf("errno: %d\n", errno);
  stat("/test", &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);

  return 0;
}
