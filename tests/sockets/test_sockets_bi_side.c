#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#if EMSCRIPTEN
#include <emscripten.h>
#endif

#define EXPECTED_BYTES 5

int sockfd = -1;

void finish(int result) {
  close(sockfd);
  exit(result);
}

void loop() {
  fd_set fdw;
  int res;

  // Make sure that sockfd has actually finished connecting
  // and is ready to read.
  FD_ZERO(&fdw);
  FD_SET(sockfd, &fdw);
  res = select(64, NULL, &fdw, NULL, NULL);
  if (res == -1) {
    perror("select failed");
    finish(EXIT_FAILURE);
  } else if (!FD_ISSET(sockfd, &fdw)) {
    return;
  }

  char data[] = "hello from the other siide\n";

  printf("send..\n");
#if TEST_FILE_OPS
  res = write(sockfd, data, sizeof(data));
#else
  res = send(sockfd, data, sizeof(data), 0);
#endif
  if (res == -1) {
    if (errno != EAGAIN) {
      perror("send error");
      finish(EXIT_FAILURE);
    }
    return;
  }

  finish(EXIT_SUCCESS);
}

int main() {
  struct sockaddr_in addr;
  int res;
#if !TEST_DGRAM
  sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
  if (sockfd == -1) {
    perror("cannot create socket");
    finish(EXIT_FAILURE);
  }
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SOCKK);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
    perror("inet_pton failed");
    finish(EXIT_FAILURE);
  }

  res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1 && errno != EINPROGRESS) {
    perror("connect failed");
    finish(EXIT_FAILURE);
  }

  emscripten_set_main_loop(loop, 0, 0);

  return EXIT_SUCCESS;
}

