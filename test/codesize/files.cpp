#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  int fd = open("some.file", O_RDWR);
  char buffer[100];
  read(fd, buffer, 1);
  write(fd, "some data", 9);
  close(fd);
}
