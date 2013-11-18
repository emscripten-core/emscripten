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
#if EMSCRIPTEN
#include <emscripten.h>
#endif

#include "test_sockets_msg.h"

typedef enum {
  MSG_READ,
  MSG_WRITE
} msg_state_t;

typedef struct {
  int fd;
} server_t;

typedef struct {
  int fd;
  struct sockaddr_in addr;
  msg_t msg;
  msg_state_t state;
  int read;
  int wrote;
} client_t;

server_t server;
client_t client;

void cleanup() {
  if (client.fd) {
    close(client.fd);
    client.fd = 0;
  }
  if (server.fd) {
    close(server.fd);
    server.fd = 0;
  }
}

void main_loop(void *arg) {
  int res;
  fd_set fdr;
  fd_set fdw;

  // see if there are any connections to accept or read / write from
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(server.fd, &fdr);
  FD_SET(server.fd, &fdw);
#if !USE_UDP
  if (client.fd) FD_SET(client.fd, &fdr);
  if (client.fd) FD_SET(client.fd, &fdw);
#endif
  res = select(64, &fdr, &fdw, NULL, NULL);
  if (res == -1) {
    perror("select failed");
    exit(EXIT_SUCCESS);
  }

#if !USE_UDP
  // for TCP sockets, we may need to accept a connection
  if (FD_ISSET(server.fd, &fdr)) {
#if TEST_ACCEPT_ADDR
    // Do an accept with non-NULL addr and addlen parameters. This tests a fix to a bug in the implementation of
    // accept which had a parameter "addrp" but used "addr" internally if addrp was set - giving a ReferenceError.
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    socklen_t addrlen = sizeof(addr);
    client.fd = accept(server.fd, (struct sockaddr *) &addr, &addrlen);
#else
    client.fd = accept(server.fd, NULL, NULL);
#endif
    assert(client.fd != -1);
  }
#endif

#if !USE_UDP
    int fd = client.fd;
#else
    int fd = server.fd;
#endif
  if (client.state == MSG_READ) {
    socklen_t addrlen;

    if (!FD_ISSET(fd, &fdr)) {
      return;
    }

    res = do_msg_read(fd, &client.msg, client.read, 0, (struct sockaddr *)&client.addr, &addrlen);
    if (res == 0) {
      // client disconnected
      memset(&client, 0, sizeof(client_t));
      return;
    } else if (res != -1) {
      client.read += res;
    }

    // once we've read the entire message, echo it back
    if (client.read >= client.msg.length) {
      client.read = 0;
      client.state = MSG_WRITE;
    }
  } else {
    if (!FD_ISSET(fd, &fdw)) {
      return;
    }

    res = do_msg_write(fd, &client.msg, client.wrote, 0, (struct sockaddr *)&client.addr, sizeof(client.addr));
    if (res == 0) {
      // client disconnected
      memset(&client, 0, sizeof(client_t));
      return;
    } else if (res != -1) {
      client.wrote += res;
    }

    if (client.wrote >= client.msg.length) {
      client.wrote = 0;
      client.state = MSG_READ;

#if CLOSE_CLIENT_AFTER_ECHO
      close(client.fd);
      memset(&client, 0, sizeof(client_t));
#endif
    }
  }
}

int main() {
  struct sockaddr_in addr;
  int res;

  atexit(cleanup);
  signal(SIGTERM, cleanup);

  memset(&server, 0, sizeof(server_t));
  memset(&client, 0, sizeof(client_t));

  // create the socket and set to non-blocking
#if !USE_UDP
  server.fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  server.fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
  if (server.fd == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  fcntl(server.fd, F_SETFL, O_NONBLOCK);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SOCKK);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
    perror("inet_pton failed");
    exit(EXIT_FAILURE);
  }

  res = bind(server.fd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

#if !USE_UDP
  res = listen(server.fd, 50);
  if (res == -1) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
#endif

#if EMSCRIPTEN
  emscripten_set_main_loop(main_loop, 60, 0);
#else
  while (1) main_loop(NULL);
#endif

  return EXIT_SUCCESS;
}
