#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

int main() {
  struct pollfd multi[5];
  multi[0].fd = open("/file", O_RDONLY, 0777);
  multi[1].fd = open("/device", O_RDONLY, 0777);
  multi[2].fd = 123;
  multi[3].fd = open("/file", O_RDONLY, 0777);
  multi[4].fd = open("/file", O_RDONLY, 0777);
  multi[0].events = POLLIN | POLLOUT | POLLNVAL | POLLERR;
  multi[1].events = POLLIN | POLLOUT | POLLNVAL | POLLERR;
  multi[2].events = POLLIN | POLLOUT | POLLNVAL | POLLERR;
  multi[3].events = 0x00;
  multi[4].events = POLLOUT | POLLNVAL | POLLERR;

  printf("ret: %d\n", poll(multi, 5, 123));
  printf("errno: %d\n", errno);
  printf("multi[0].revents: %d\n", multi[0].revents == (POLLIN | POLLOUT));
  printf("multi[1].revents: %d\n", multi[1].revents == (POLLIN | POLLOUT));
  printf("multi[2].revents: %d\n", multi[2].revents == POLLNVAL);
  printf("multi[3].revents: %d\n", multi[3].revents == 0);
  printf("multi[4].revents: %d\n", multi[4].revents == POLLOUT);

  return 0;
}
