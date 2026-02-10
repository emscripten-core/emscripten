#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main() {
  int res = symlink("link_target", "link_source");
  printf("link result: %d\n", res);
  int src_fd = open("link_source", O_CREAT | O_WRONLY, 0777);
  printf("source_fd: %d, errno: %d %s\n", src_fd, errno, strerror(errno));
  write(src_fd, "abc", 3);
  close(src_fd);
  {
    int target_fd = open("link_target", O_RDONLY);
    printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
    char buf[10];
    memset(buf, 0, 10);
    size_t r = read(target_fd, buf, 10);
    assert(r == 3);
    printf("buf: '%s'\n", buf);
    close(target_fd);
  }
  {
    int target_fd = open("link_source", O_RDONLY);
    printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
    char buf[10];
    memset(buf, 0, 10);
    size_t r = read(target_fd, buf, 10);
    assert(r == 3);
    printf("buf: '%s'\n", buf);
    close(target_fd);
  }
}
