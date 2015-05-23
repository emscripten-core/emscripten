#include <stdio.h>
#include <emscripten.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int result = 1;

int main() {
  struct stat st;
  int fd;

  // We first open and close an empty file in the directory and leave, which
  // should trigger syncfs() on exit. We will then verify the existance of the
  // file on the !FIRST run.

#if FIRST

  if ((stat("/working1/empty.txt", &st) != -1) || (errno != ENOENT))
    result = -1000 - errno;
  fd = open("/working1/empty.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -2000 - errno;
  else if (close(fd) != 0)
    result = -3000 - errno;

#else

  if (stat("/working1/empty.txt", &st) != 0)
    result = -4000 - errno;
  if (unlink("/working1/empty.txt") != 0)
    result = -5000 - errno;

#endif

  REPORT_RESULT();
  return 0;
}
