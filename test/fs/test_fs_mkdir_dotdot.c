#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

int main() {
  assert(mkdir("test", 0777) == 0);
  assert(mkdir("test/a", 0777) == 0);
  assert(mkdir("test/a/..", 0777) == -1);
  printf("error: %s\n", strerror(errno));
  assert(errno == EEXIST);
  assert(mkdir("test/a/.", 0777) == -1);
  printf("error: %s\n", strerror(errno));
  assert(errno == EEXIST);
  assert(mkdir("test/a/b/..", 0777) == -1);
  printf("error: %s\n", strerror(errno));
  assert(errno == ENOENT);
  assert(mkdir("test/a/b/.", 0777) == -1);
  printf("error: %s\n", strerror(errno));
  assert(errno == ENOENT);
  printf("success\n");
}
