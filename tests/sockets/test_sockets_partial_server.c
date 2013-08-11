#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#if EMSCRIPTEN
#include <emscripten.h>
#endif

int serverfd = -1;
int clientfd = -1;

// csock.send("\x09\x01\x02\x03\x04\x05\x06\x07\x08\x09")
// csock.send("\x08\x01\x02\x03\x04\x05\x06\x07\x08")
// csock.send("\x07\x01\x02\x03\x04\x05\x06\x07")
// csock.send("\x06\x01\x02\x03\x04\x05\x06")
// csock.send("\x05\x01\x02\x03\x04\x05")
// csock.send("\x04\x01\x02\x03\x04")
// csock.send("\x03\x01\x02\x03")
// csock.send("\x02\x01\x02")
// csock.send("\x01\x01")

void do_send(int sockfd) {
  static char* buffers[] = {
    "\x09\x01\x02\x03\x04\x05\x06\x07\x08\x09\0",
    "\x08\x01\x02\x03\x04\x05\x06\x07\x08\0",
    "\x07\x01\x02\x03\x04\x05\x06\x07\0",
    "\x06\x01\x02\x03\x04\x05\x06\0",
    "\x05\x01\x02\x03\x04\x05\0",
    "\x04\x01\x02\x03\x04\0",
    "\x03\x01\x02\x03\0",
    "\x02\x01\x02\0",
    "\x01\x01\0"
  };

  int i;
  int res;
  char *buffer;
  struct sockaddr_in addr;
  socklen_t addrlen;

  for (i = 0; i < sizeof(buffers) / sizeof(char*); i++) {
    buffer = buffers[i];

    res = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
      perror("send failed");
      exit(EXIT_FAILURE);
    }
    printf("sent \"%s\" (%d bytes)\n", buffer, res);
  }

  exit(EXIT_SUCCESS);
}

void iter(void *arg) {
  int res;
  fd_set fdr;
  fd_set fdw;

  // see if there are any connections to accept / write to
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(serverfd, &fdr);
  if (clientfd != -1) FD_SET(clientfd, &fdw);
  res = select(64, &fdr, &fdw, NULL, NULL);
  if (res == -1) {
    perror("select failed");
    exit(EXIT_SUCCESS);
  }

  if (FD_ISSET(serverfd, &fdr)) {
    printf("accepted someone\n");
    clientfd = accept(serverfd, NULL, NULL);
    assert(clientfd != -1);
  }

  if (FD_ISSET(clientfd, &fdw)) {
    do_send(clientfd);
  }
}

int main() {
  struct sockaddr_in addr;
  int res;

  // create the socket
  serverfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverfd == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  fcntl(serverfd, F_SETFL, O_NONBLOCK);

  // bind and listen to the supplied port
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SOCKK);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
    perror("inet_pton failed");
    exit(EXIT_FAILURE);
  }

  res = bind(serverfd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  res = listen(serverfd, 50);
  if (res == -1) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

#if EMSCRIPTEN
  emscripten_set_main_loop(iter, 60, 0);
#else
  while (1) iter(NULL);
#endif

  return EXIT_SUCCESS;
}
