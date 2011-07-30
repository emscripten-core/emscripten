#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  char buffer[256];
  int d = open("/device", O_RDWR);
  int f = open("/", O_RDONLY);
  char* result;

  result = ctermid(buffer);
  if (result) {
    printf("ctermid: %s\n", result);
  } else {
    printf("ctermid errno: %d\n", errno);
    errno = 0;
  }

  if (ttyname_r(d, buffer, 256) == 0) {
    printf("ttyname_r(d, ..., 256): %s\n", buffer);
  } else {
    printf("ttyname_r(d, ..., 256) errno: %d\n", errno);
    errno = 0;
  }

  if (ttyname_r(d, buffer, 2) == 0) {
    printf("ttyname_r(d, ..., 2): %s\n", buffer);
  } else {
    printf("ttyname_r(d, ..., 2) errno: %d\n", errno);
    errno = 0;
  }

  result = ttyname(d);
  if (result) {
    printf("ttyname(d): %s\n", result);
  } else {
    printf("ttyname(d) errno: %d\n", errno);
    errno = 0;
  }

  result = ttyname(f);
  if (result) {
    printf("ttyname(f): %s\n", result);
  } else {
    printf("ttyname(f) errno: %d\n", errno);
    errno = 0;
  }

  return 0;
}
