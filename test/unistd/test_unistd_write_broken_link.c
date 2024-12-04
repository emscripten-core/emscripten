#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main() {
  char* p1 = "link_source";
  char* p2 = "link_target";

  int res = symlink(p2, p1);
  printf("link result: %d\n", res);
  int src_fd = open(p1, O_CREAT | O_WRONLY, 0777);
  printf("source_fd: %d, errno: %d %s\n", src_fd, errno, strerror(errno));
  write(src_fd, "abc", 3);
  close(src_fd);
  {
    int target_fd = open(p2, O_RDONLY);
    printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
    char buf[10];
    read(target_fd, buf, 10);
    printf("buf: '%s'\n", buf);
    close(target_fd);
  }
  {
    int target_fd = open(p1, O_RDONLY);
    printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
    char buf[10];
    read(target_fd, buf, 10);
    printf("buf: '%s'\n", buf);
    close(target_fd);
  }
}
