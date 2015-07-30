#include <stdio.h>
#include <emscripten.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int result = 1;

int main() {
  int fd;

  // We first make sure the file doesn't currently exist.
  // We then write a file, call fsync, and close the file,
  // to make sure synchronous calls to resume does not throw.

  struct stat st;

  // a file whose contents are just 'az'
  if ((stat("/wakaka.txt", &st) != -1) || (errno != ENOENT))
    result = -1000 - errno;
  fd = open("/wakaka.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -2000 - errno;
  else
  {
    if (write(fd,"az",2) != 2)
      result = -3000 - errno;

    if (fsync(fd) != 0)
      result = -4000 - errno;

    if (close(fd) != 0)
      result = -5000 - errno;
  }

  REPORT_RESULT();
}
