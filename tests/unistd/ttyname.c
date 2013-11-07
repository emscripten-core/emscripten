#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  int err;
  int stdin, null, dev;
  char buffer[256];
  char* result;

  stdin = open("/dev/stdin", O_RDONLY);
  null = open("/dev/null", O_RDONLY);
  dev = open("/dev", O_RDONLY);

  result = ctermid(buffer);
  assert(!strcmp(result, "/dev/tty"));

  // strstr instead of strcmp as native code may
  // be using a virtual console (e.g. /dev/tty02)
  err = ttyname_r(stdin, buffer, 256);
  assert(!err);
  assert(strstr(buffer, "/dev/tty"));

  err = ttyname_r(stdin, buffer, 2);
  assert(err == ERANGE);

  result = ttyname(stdin);
  assert(strstr(result, "/dev/tty"));

  result = ttyname(null);
  assert(!result);

  result = ttyname(dev);
  assert(!result);

  puts("success");

  return EXIT_SUCCESS;
}
