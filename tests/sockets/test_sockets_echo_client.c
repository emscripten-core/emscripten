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
#ifdef __EMSCRIPTEN__
  REPORT_RESULT();
  emscripten_force_exit(result);
#else
  exit(result);
#endif
}

void main_loop() {
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

#if !TEST_DGRAM
    // as a test, confirm with ioctl that we have data available
    // after selecting
    int available;
    res = ioctl(server.fd, FIONREAD, &available);
    assert(res != -1);
    assert(available);
#endif

    res = do_msg_read(server.fd, &server.msg, echo_read, 0, NULL, NULL);
    if (res == -1) {
      return;
    } else if (res == 0) {
      perror("server closed");
      finish(EXIT_FAILURE);
    }

    echo_read += res;

    // once we've read the entire message, validate it
    if (echo_read >= server.msg.length) {
      assert(!strcmp(server.msg.buffer, MESSAGE));
      finish(EXIT_SUCCESS);
    }
  }

  if (server.state == MSG_WRITE) {
    if (!FD_ISSET(server.fd, &fdw)) {
      return;
    }

    res = do_msg_write(server.fd, &echo_msg, echo_wrote, 0, NULL, 0);
    if (res == -1) {
      return;
    } else if (res == 0) {
      perror("server closed");
      finish(EXIT_FAILURE);
    }

    echo_wrote += res;

    // once we're done writing the message, read it back
    if (echo_wrote >= echo_msg.length) {
      server.state = MSG_READ;
    }
  }
}

// The callbacks for the async network events have a different signature than from
// emscripten_set_main_loop (they get passed the fd of the socket triggering the event).
// In this test application we want to try and keep as much in common as the timed loop
// version but in a real application the fd can be used instead of needing to select().
void async_main_loop(int fd, void* userData) {
  printf("%s callback\n", userData);
  main_loop();
}

void error_callback(int fd, int err, const char* msg, void* userData) {
  int error;
  socklen_t len = sizeof(error);

  int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
  printf("%s callback\n", userData);
  printf("error message: %s\n", msg);

  if (err == error) {
    finish(EXIT_SUCCESS);
  } else {
    finish(EXIT_FAILURE);
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

  echo_read = 0;
  echo_wrote = 0;

  // create the socket and set to non-blocking
#if !TEST_DGRAM
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

  {
    int z;
    struct sockaddr_in adr_inet;
    socklen_t len_inet = sizeof adr_inet;
    z = getsockname(server.fd, (struct sockaddr *)&adr_inet, &len_inet);
    if (z != 0) {
      perror("getsockname");
      finish(EXIT_FAILURE);
    }
    char buffer[1000];
    sprintf(buffer, "%s:%u", inet_ntoa(adr_inet.sin_addr), (unsigned)ntohs(adr_inet.sin_port));
    // TODO: This is not the correct result: We should have a auto-bound address
    char *correct = "0.0.0.0:0";
    printf("got (expected) socket: %s (%s), size %d (%d)\n", buffer, correct, strlen(buffer), strlen(correct));
    assert(strlen(buffer) == strlen(correct));
    assert(strcmp(buffer, correct) == 0);
  }

  {
    int z;
    struct sockaddr_in adr_inet;
    socklen_t len_inet = sizeof adr_inet;
    z = getpeername(server.fd, (struct sockaddr *)&adr_inet, &len_inet);
    if (z != 0) {
      perror("getpeername");
      finish(EXIT_FAILURE);
    }
    char buffer[1000];
    sprintf(buffer, "%s:%u", inet_ntoa(adr_inet.sin_addr), (unsigned)ntohs(adr_inet.sin_port));
    char correct[1000];
    sprintf(correct, "127.0.0.1:%u", SOCKK);
    printf("got (expected) socket: %s (%s), size %d (%d)\n", buffer, correct, strlen(buffer), strlen(correct));
    assert(strlen(buffer) == strlen(correct));
    assert(strcmp(buffer, correct) == 0);
  }

#ifdef __EMSCRIPTEN__
#if TEST_ASYNC
  // The first parameter being passed is actually an arbitrary userData pointer
  // for simplicity this test just passes a basic char*
  emscripten_set_socket_error_callback("error", error_callback);
  emscripten_set_socket_open_callback("open", async_main_loop);
  emscripten_set_socket_message_callback("message", async_main_loop);
#else
  emscripten_set_main_loop(main_loop, 60, 0);
#endif
#else
  while (1) main_loop();
#endif

  return EXIT_SUCCESS;
}
