/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Under -sNODERAWSOCKETS TCP binds eagerly and synchronously, so: a client that
 * bind()s an explicit source port has it honored by connect() (getsockname
 * reports it), and a bind() that conflicts with a port already in use fails
 * synchronously with EADDRINUSE - exactly where POSIX reports it - rather than
 * being deferred. argv: <server port> <free source port>. The same code builds
 * and runs natively against the host stack.
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
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static int client_fd = -1;
static struct sockaddr_in dest;
static uint16_t src_port = 0;
static bool connected = false;
static bool ping_sent = false;

static void finish(int result) {
  printf(result == 0 ? "CLIENT BIND PASS\n" : "CLIENT BIND FAIL\n");
  if (client_fd >= 0) close(client_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
  if (result != 0) abort();
#else
  exit(result);
#endif
}

static struct sockaddr_in loopback(uint16_t port) {
  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  a.sin_port = htons(port);
  return a;
}

static void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(client_fd, &fdr);
  FD_SET(client_fd, &fdw);
  select(client_fd + 1, &fdr, &fdw, NULL, &tv);

  if (!connected && FD_ISSET(client_fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &err, &l);
    if (err != 0) {
      printf("connect failed: %s\n", strerror(err));
      finish(1);
      return;
    }
    connected = true;

    // The explicitly bound source port must be the one in use.
    struct sockaddr_in sa;
    socklen_t sl = sizeof(sa);
    assert(getsockname(client_fd, (struct sockaddr*)&sa, &sl) == 0);
    if (sa.sin_port != htons(src_port)) {
      printf("source port not honored: bound %u, got %u\n", src_port, ntohs(sa.sin_port));
      finish(1);
      return;
    }
  }

  if (connected && !ping_sent && FD_ISSET(client_fd, &fdw)) {
    if (send(client_fd, "ping", 4, 0) == 4) ping_sent = true;
  }

  if (ping_sent && FD_ISSET(client_fd, &fdr)) {
    char buf[4];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n == 4 && memcmp(buf, "ping", 4) == 0) {
      finish(0);
    } else if (n == 0) {
      printf("peer closed unexpectedly\n");
      finish(1);
    }
  }
}

int main(int argc, char** argv) {
  assert(argc > 2 && "usage: test_tcp_client_bind <server port> <source port>");
  int port = atoi(argv[1]);
  src_port = (uint16_t)atoi(argv[2]);

  // A bind() to the port the echo server is already listening on must fail
  // synchronously with EADDRINUSE (eager bind, no deferral).
  int busy = socket(AF_INET, SOCK_STREAM, 0);
  assert(busy >= 0);
  struct sockaddr_in inuse = loopback((uint16_t)port);
  int br = bind(busy, (struct sockaddr*)&inuse, sizeof(inuse));
  if (!(br == -1 && errno == EADDRINUSE)) {
    printf("expected EADDRINUSE binding a busy port, got r=%d errno=%d\n", br, errno);
    finish(1);
    return 0;
  }
  close(busy);

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  fcntl(client_fd, F_SETFL, O_NONBLOCK);

  // Bind the client to the chosen free source port before connecting.
  struct sockaddr_in src = loopback(src_port);
  assert(bind(client_fd, (struct sockaddr*)&src, sizeof(src)) == 0);

  dest = loopback((uint16_t)port);
  int r = connect(client_fd, (struct sockaddr*)&dest, sizeof(dest));
  if (r != 0 && errno != EINPROGRESS) {
    perror("connect");
    return 1;
  }

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
