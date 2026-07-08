/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Self-contained TCP loopback peek test. A listener and a client live in one
 * process. The client sends one message; the server recv()s it with MSG_PEEK
 * (which must leave the data buffered), asserts the socket is still readable,
 * then recv()s it again without MSG_PEEK and gets the same bytes. This is plain
 * POSIX and also builds/runs natively against the host stack.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define MSG "peekme"
#define MSGLEN 6

int listen_fd = -1;
int client_fd = -1;
int peer_fd = -1; // accepted (server-side) connection
struct sockaddr_in dest;
bool connected = false;
bool sent = false;

void set_nonblocking(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);
}

void test_success(void) {
  printf("TCP PEEK PASS\n");
  if (listen_fd >= 0) close(listen_fd);
  if (client_fd >= 0) close(client_fd);
  if (peer_fd >= 0) close(peer_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void start_client(void) {
  if (client_fd >= 0) close(client_fd);
  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  set_nonblocking(client_fd);
  connected = false;
  sent = false;
  int r = connect(client_fd, (struct sockaddr*)&dest, sizeof(dest));
  assert((r == 0 || errno == EINPROGRESS) && "connect");
}

int readable(int fd) {
  fd_set fdr;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_SET(fd, &fdr);
  return select(fd + 1, &fdr, NULL, NULL, &tv) > 0 && FD_ISSET(fd, &fdr);
}

void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(listen_fd, &fdr);
  FD_SET(client_fd, &fdw);
  if (peer_fd >= 0) FD_SET(peer_fd, &fdr);
  select(64, &fdr, &fdw, NULL, &tv);

  // server: accept the incoming connection
  if (peer_fd < 0 && FD_ISSET(listen_fd, &fdr)) {
    peer_fd = accept(listen_fd, NULL, NULL);
    if (peer_fd >= 0) set_nonblocking(peer_fd);
  }

  // client: connect completion (retry while the listener is coming up)
  if (!connected && FD_ISSET(client_fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &err, &l);
    if (err == ECONNREFUSED || err == ECONNRESET) {
      start_client();
      return;
    }
    assert(err == 0 && "connect failed");
    connected = true;
  }

  // client: send the message once connected
  if (connected && !sent && FD_ISSET(client_fd, &fdw)) {
    if (send(client_fd, MSG, MSGLEN, 0) == MSGLEN) sent = true;
  }

  // server: peek, then read - both must return the same bytes
  if (peer_fd >= 0 && FD_ISSET(peer_fd, &fdr)) {
    char buf[MSGLEN];

    memset(buf, 0, sizeof(buf));
    ssize_t n = recv(peer_fd, buf, sizeof(buf), MSG_PEEK);
    assert(n == MSGLEN && "peek length");
    assert(memcmp(buf, MSG, MSGLEN) == 0 && "peek data");

    // The peek must not have consumed the data: still readable.
    assert(readable(peer_fd) && "readable after peek");

    memset(buf, 0, sizeof(buf));
    n = recv(peer_fd, buf, sizeof(buf), 0);
    assert(n == MSGLEN && "recv length");
    assert(memcmp(buf, MSG, MSGLEN) == 0 && "recv data");

    test_success();
  }
}

int main(void) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0); // ephemeral
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("bind");
    return 1;
  }
  if (listen(listen_fd, 4) != 0) {
    perror("listen");
    return 1;
  }

  struct sockaddr_in la;
  socklen_t ll = sizeof(la);
  if (getsockname(listen_fd, (struct sockaddr*)&la, &ll) != 0) {
    perror("getsockname");
    return 1;
  }
  set_nonblocking(listen_fd);

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = la.sin_port;
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);
  start_client();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (1) {
    main_loop();
    usleep(1000);
  }
#endif
  return 0;
}
