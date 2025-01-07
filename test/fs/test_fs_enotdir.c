#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

int main() {
  {
    int src_fd = open("file", O_CREAT | O_WRONLY, 0777);
    assert(src_fd >= 0);
    assert(close(src_fd) == 0);
  }
  {
    assert(mkdir("file/blah", 0777) == -1);
    assert(errno == ENOTDIR);
  }
  {
    assert(open("./does-not-exist/", O_CREAT, 0777) == -1);
    assert(errno == EISDIR);
  }
  printf("success\n");
}
