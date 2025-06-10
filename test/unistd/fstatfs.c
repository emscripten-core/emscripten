#include <assert.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  struct statfs buf;

  int rtn;
  assert(fstatfs(STDOUT_FILENO, &buf) == 0);
  printf("f_type: %ld\n", buf.f_type);

  int f = open("file", O_RDWR | O_CREAT);
  assert(fstatfs(f, &buf) == 0);
  printf("f_type: %ld\n", buf.f_type);

  assert(statfs("file", &buf) == 0);
  printf("f_type: %ld\n", buf.f_type);
  return 0;
}
