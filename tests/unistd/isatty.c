#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  printf("read: %d\n", isatty(open("/read", O_RDONLY)));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("write: %d\n", isatty(open("/write", O_WRONLY)));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("all: %d\n", isatty(open("/all", O_RDONLY)));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("folder: %d\n", isatty(open("/folder", O_RDONLY)));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("file: %d\n", isatty(open("/file", O_RDONLY)));
  printf("errno: %d\n", errno);
  errno = 0;

  return 0;
}
