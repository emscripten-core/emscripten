#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

static void create_file(const char *path, const char *buffer) {
  printf("creating: %s\n", path);
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
  printf("error: %s\n", strerror(errno));
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

int main() {
  create_file("a", "abc");
  create_file("b", "xyz");
  assert(rename("a", "b") == 0);
  assert(unlink("b") == 0);
  create_file("b", "xyz");
  printf("success\n");
}
