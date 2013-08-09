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
#if EMSCRIPTEN
#include <emscripten.h>
#endif

#define EXPECTED_BYTES 5

int sockfd = -1;

void finish(int result) {
  close(sockfd);
#if EMSCRIPTEN
  REPORT_RESULT();
#endif
  exit(result);
}

void iter(void *arg) {  
  static char readbuf[1024];
  static int readPos = 0;
  
  fd_set sett;
  FD_ZERO(&sett);
  FD_SET(sockfd, &sett);
  
  int res = select(64, &sett, NULL, NULL, NULL);
  
  if (res == -1) {
    perror("select failed");
    finish(EXIT_FAILURE);
  } else if (res == 0) {
    return;
  } else if (res > 0) {
    assert(FD_ISSET(sockfd, &sett));
    
    int bytesRead = recv(sockfd, readbuf+readPos, 7-readPos, 0);
    if (bytesRead == -1) {
      if (errno != EAGAIN) {
        perror("recv error");
        finish(EXIT_FAILURE);
      }
      // try again
      return;
    }

    if (readPos < 7) {
      readPos += bytesRead;
    } else {
      if (!bytesRead) {
        perror("Connection to websocket server was closed as expected");
        finish(266);
      } else {
        perror("Connection to websocket server was not closed");
        finish(EXIT_FAILURE);
      }
    }
  }
  
  return;
}

// Scenario: the server sends data and closes the connection after 7 bytes. 
// This test should provoke the situation in which the underlying 
// tcp connection has been torn down already but there is still data 
// in the queue. The select call has to succeed as long the queue
// still contains data and only then start to throw errors.
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

#if EMSCRIPTEN
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (1) iter(NULL);
#endif

  return EXIT_FAILURE;
}

