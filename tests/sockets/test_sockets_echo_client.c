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

#include "test_sockets_msg.h"

// message to send to the server
#ifndef MESSAGE
#define MESSAGE "pingtothepong"
#endif

typedef enum {
  MSG_READ,
  MSG_WRITE
} msg_state_t;

typedef struct {
  int fd;
  msg_t msg;
  msg_state_t state;
} server_t;

server_t server;
msg_t echo_msg;
int echo_read;
int echo_wrote;

void finish(int result) {
  if (server.fd) {
    close(server.fd);
    server.fd = 0;
  }
#if EMSCRIPTEN
  REPORT_RESULT();
#endif
  exit(result);
}

void main_loop(void *arg) {
  static char out[1024*2];
  static int pos = 0;
  fd_set fdr;
  fd_set fdw;
  int res;

  // make sure that server.fd is ready to read / write
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(server.fd, &fdr);
  FD_SET(server.fd, &fdw);
  res = select(64, &fdr, &fdw, NULL, NULL);
  if (res == -1) {
    perror("select failed");
    finish(EXIT_FAILURE);
  }

  if (server.state == MSG_READ) {
    if (!FD_ISSET(server.fd, &fdr)) {
      return;
    }

    // as a test, confirm with ioctl that we have data available
    // after selecting
    int available;
    res = ioctl(server.fd, FIONREAD, &available);
    assert(res != -1);
    assert(available);

    res = do_msg_read(server.fd, &server.msg, echo_read, 0, NULL, NULL);
    if (res == 0) {
      perror("server closed");
      finish(EXIT_FAILURE);
    } else if (res != -1) {
      echo_read += res;
    }

    // once we've read the entire message, validate it
    if (echo_read >= server.msg.length) {
      assert(!strcmp(server.msg.buffer, MESSAGE));
      finish(EXIT_SUCCESS);
    }
  } else {
    if (!FD_ISSET(server.fd, &fdw)) {
      return;
    }

    res = do_msg_write(server.fd, &echo_msg, echo_wrote, 0, NULL, 0);
    if (res == 0) {
      perror("server closed");
      finish(EXIT_FAILURE);
    } else if (res != -1) {
      echo_wrote += res;
    }

    // once we're done writing the message, read it back
    if (echo_wrote >= echo_msg.length) {
      server.state = MSG_READ;
    }
  }
}

int main() {
  struct sockaddr_in addr;
  int res;

  memset(&server, 0, sizeof(server_t));
  server.state = MSG_WRITE;

  // setup the message we're going to echo
  memset(&echo_msg, 0, sizeof(msg_t));
  echo_msg.length = strlen(MESSAGE) + 1;
  echo_msg.buffer = malloc(echo_msg.length);
  strncpy(echo_msg.buffer, MESSAGE, echo_msg.length);

  // create the socket and set to non-blocking
#if !USE_UDP
  server.fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  server.fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
  if (server.fd == -1) {
    perror("cannot create socket");
    finish(EXIT_FAILURE);
  }
  fcntl(server.fd, F_SETFL, O_NONBLOCK);

  // connect the socket
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SOCKK);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
    perror("inet_pton failed");
    finish(EXIT_FAILURE);
  }
  
  res = connect(server.fd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1 && errno != EINPROGRESS) {
    perror("connect failed");
    finish(EXIT_FAILURE);
  }

#if EMSCRIPTEN
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (1) main_loop(NULL);
#endif

  return EXIT_SUCCESS;
}