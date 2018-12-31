#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int serverfd = 0;
int clientfd = 0;

void cleanup_client() {
  if (clientfd) {
    close(clientfd);
    clientfd = 0;
  }
}

void cleanup() {
  if (serverfd) {
    close(serverfd);
    serverfd = 0;
  }
  cleanup_client();
}

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
  socklen_t addrlen;

  for (i = 0; i < sizeof(buffers) / sizeof(char*); i++) {
    buffer = buffers[i];

    res = send(sockfd, buffer, strlen(buffer), 0);
    if (res == -1) {
      perror("send failed");
      return;
    }
    printf("sent \"%s\" (%d bytes)\n", buffer, res);
  }
}

void iter() {
  int res;
  fd_set fdr;
  fd_set fdw;

  // see if there are any connections to accept / write to
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(serverfd, &fdr);
  if (clientfd) FD_SET(clientfd, &fdw);
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
    cleanup_client();
  }
}

int main() {
  struct sockaddr_in addr;
  int res;

  atexit(cleanup);
  signal(SIGTERM, cleanup);

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

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(iter, 60, 0);
#else
  while (1) iter();
#endif

  return EXIT_SUCCESS;
}
