#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  {
    int src_fd = open("file", O_CREAT | O_WRONLY, 0777);
    close(src_fd);
  }
  {
    int target_fd = mkdir("file/blah", 0777);
    printf("target_fd: %d, errno: %d %s\n", target_fd, errno, strerror(errno));
  }
}
