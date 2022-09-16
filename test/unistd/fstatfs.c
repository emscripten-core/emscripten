#include <assert.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <unistd.h>

int main() {
  struct statfs buf;
  int rtn = fstatfs(STDOUT_FILENO, &buf);
  assert(rtn == 0);
  printf("f_type: %ld\n", buf.f_type);
  return 0;
}
