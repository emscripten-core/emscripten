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
#include <sys/ioctl.h>
#include <assert.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int sockfd = -1;
int sum = 0;

void finish(int result) {
  close(sockfd);
#ifdef __EMSCRIPTEN__
  REPORT_RESULT();
  emscripten_force_exit(result);
#else
  exit(result);
#endif
}

void iter() {
  char buffer[1024];
  char packetLength;
  fd_set fdr;
  int i;
  int res;

  // make sure that sockfd is ready to read
  FD_ZERO(&fdr);
  FD_SET(sockfd, &fdr);
  res = select(64, &fdr, NULL, NULL, NULL);
  if (res == -1) {
    perror("select failed");
    finish(EXIT_FAILURE);
  } else if (!FD_ISSET(sockfd, &fdr)) {
    return;
  }

  res = recv(sockfd, buffer, 1, 0);
  if (res == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return; //try again
    }

    perror("unexcepted end of data");
    finish(EXIT_FAILURE);
  }

  if (res != 1) {
    fprintf(stderr, "should read 1 byte, got: %d, sum %d so far\n", res, sum);
    finish(EXIT_FAILURE);
  }

  packetLength = buffer[0];
  res = recv(sockfd, buffer, packetLength, 0);

  printf("got %d,%d\n", res, packetLength);

  if (res != packetLength) {
    fprintf(stderr, "lost packet data, expected: %d readed: %d", packetLength, res);
    finish(EXIT_FAILURE);
  }

  for (i = 0; i < packetLength; ++i) {
    if (buffer[i] != i+1) {
      fprintf(stderr, "packet corrupted, expected: %d, actual: %d", i+1, buffer[i]);
      finish(EXIT_FAILURE);
    }

    sum += buffer[i];
  }

  if (packetLength == buffer[0]) { // \x01\x01 - end marker
    printf("sum: %d\n", sum);
    finish(sum);
  }
}

int main() {
  struct sockaddr_in addr;
  int res;

  sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
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

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (1) iter();
#endif

  return EXIT_SUCCESS;
}

