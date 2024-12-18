#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

// TODO: Combine this with test_fcntl_open.c. First requires fixing
// test_fcntl_open.c on noderawfs.
int main() {
  int res = open("a", O_CREAT, 0);
  printf("error: %s\n", strerror(errno));
  assert(res >= 0);
  struct stat st;
  assert(stat("a", &st) == 0);
  assert((st.st_mode & 0777) == 0);
  printf("success\n");
}
