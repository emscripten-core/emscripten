#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main() {
  assert(mkdir("path", 0777) == 0);
  assert(close(open("path/file", O_CREAT | O_WRONLY, 0644)) == 0);
  {
    struct stat st;
    assert(stat("path", &st) == 0);
    assert(st.st_mode = 0777);
  }
  {
    struct stat st;
    assert(stat("path/nosuchfile", &st) == -1);
    printf("info: errno=%d %s\n", errno, strerror(errno));
    assert(errno == ENOENT);
  }
  {
    struct stat st;
    assert(stat("path/file", &st) == 0);
    assert(st.st_mode = 0666);
  }
  {
    struct stat st;
    assert(stat("path/file/impossible", &st) == -1);
    printf("info: errno=%d %s\n", errno, strerror(errno));
    assert(errno == ENOTDIR);
  }
  {
    struct stat st;
    assert(lstat("path/file/impossible", &st) == -1);
    printf("info: errno=%d %s\n", errno, strerror(errno));
    assert(errno == ENOTDIR);
  }
  return 0;
}
