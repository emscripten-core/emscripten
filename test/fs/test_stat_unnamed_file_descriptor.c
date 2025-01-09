#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include "stdio.h"

int main() {
  int fd = open("file.txt", O_RDWR | O_CREAT, 0666);
  unlink("file.txt");
  int res;
  struct stat buf;
  res = fstat(fd, &buf);
  assert(res == 0);
  assert(buf.st_atime > 1000000000);
  res = fchmod(fd, 0777);
  assert(res == 0);
  res = ftruncate(fd, 10);
  assert(res == 0);
  printf("success\n");
}
