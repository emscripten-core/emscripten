#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define EXPECTED_BYTES 5

int sockfd = -1;

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
  static int retries = 0;

  fd_set sett;
  FD_ZERO(&sett);
  FD_SET(sockfd, &sett);
  
  // currently, we've connected to a closed server port.
  // the initial async connect "succeeded" and select
  // should say that the socket is ready for a non-blocking
  // read, however, the read should be 0 sized signalling
  // that the remote end has closed.
  int handles = select(64, &sett, NULL, NULL, NULL);
  if (handles == -1) {
    perror("select failed");
    finish(EXIT_FAILURE);
  }

  if (FD_ISSET(sockfd, &sett)) {
    char buffer[1024];
    int n = recv(sockfd, buffer, sizeof(buffer), 0);
    if (n == -1 && retries++ > 10) {
      perror("recv failed");
      finish(EXIT_FAILURE);
    } else if (!n) {
      perror("Connection to websocket server failed as expected.");
      finish(266);
    }
  }
}

// This is for testing a websocket connection to a closed server port.
// The connect call will succeed (due to the asynchronous websocket
// behavior) but once the underlying websocket system realized that 
// the connection cannot be established, the next select call will fail.
int main() {
  struct sockaddr_in addr;
  int res;

  sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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

  // This call should succeed (even if the server port is closed)
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
  
  return EXIT_FAILURE;
}

