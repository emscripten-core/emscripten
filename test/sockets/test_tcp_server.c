/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Self-contained TCP loopback accept+echo. A listener and a client live in one
 * process, both non-blocking, driven by select in the main loop. Exercises
 * bind(:0) + getsockname (synchronous ephemeral port), listen, accept,
 * non-blocking connect, send and recv. This is plain POSIX and also builds and
 * runs natively, so the same code can be checked against the host stack.
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

int listen_fd = -1;
int client_fd = -1;
int peer_fd = -1; // accepted (server-side) connection
struct sockaddr_in dest;
bool connected = false;
bool ping_sent = false;
bool pong_sent = false;

void set_nonblocking(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);
}

void test_success(void) {
  printf("TCP SERVER PASS\n");
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
  ping_sent = false;
  int r = connect(client_fd, (struct sockaddr*)&dest, sizeof(dest));
  assert((r == 0 || errno == EINPROGRESS) && "connect");
}

void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(listen_fd, &fdr);
  FD_SET(client_fd, &fdr);
  FD_SET(client_fd, &fdw);
  if (peer_fd >= 0) FD_SET(peer_fd, &fdr);
  select(64, &fdr, &fdw, NULL, &tv);

  // server: accept the incoming connection
  if (peer_fd < 0 && FD_ISSET(listen_fd, &fdr)) {
    struct sockaddr_in ca;
    socklen_t cl = sizeof(ca);
    peer_fd = accept(listen_fd, (struct sockaddr*)&ca, &cl);
    if (peer_fd >= 0) {
      set_nonblocking(peer_fd);
      printf("accepted from %s:%u\n", inet_ntoa(ca.sin_addr), (unsigned)ntohs(ca.sin_port));
    }
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
    printf("connected\n");
  }

  // client: send ping
  if (connected && !ping_sent && FD_ISSET(client_fd, &fdw)) {
    if (send(client_fd, "ping", 4, 0) == 4) ping_sent = true;
  }

  // server: echo ping -> pong
  if (peer_fd >= 0 && !pong_sent && FD_ISSET(peer_fd, &fdr)) {
    char buf[4];
    ssize_t n = recv(peer_fd, buf, sizeof(buf), 0);
    if (n == 4 && memcmp(buf, "ping", 4) == 0) {
      send(peer_fd, "pong", 4, 0);
      pong_sent = true;
    }
  }

  // client: receive pong
  if (ping_sent && FD_ISSET(client_fd, &fdr)) {
    char buf[4];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n == 4 && memcmp(buf, "pong", 4) == 0) {
      test_success();
    } else if (n == 0) {
      assert(false && "peer closed unexpectedly");
    }
  }
}

int main(void) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);

#ifndef NO_EXPLICIT_BIND
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0); // ephemeral
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("bind");
    return 1;
  }
#endif
  // With NO_EXPLICIT_BIND, listen() must auto-bind an ephemeral port (POSIX),
  // and getsockname() below must still report it.
  if (listen(listen_fd, 4) != 0) {
    perror("listen");
    return 1;
  }

  // The OS-assigned ephemeral port must be readable synchronously.
  struct sockaddr_in la;
  socklen_t ll = sizeof(la);
  if (getsockname(listen_fd, (struct sockaddr*)&la, &ll) != 0) {
    perror("getsockname");
    return 1;
  }
  assert(ntohs(la.sin_port) != 0);
  printf("listening on 127.0.0.1:%u\n", (unsigned)ntohs(la.sin_port));
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
