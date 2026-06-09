/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Outgoing TCP echo client. We connect to a loopback echo server started by
 * the test harness, whose port arrives as argv[1], then do a non-blocking
 * connect, send "ping" and recv the echo, all driven by select in the main
 * loop. This is plain POSIX and also builds and runs natively, so the same
 * code can be checked against the host stack.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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

int client_fd = -1;
struct sockaddr_in dest;
bool connected = false;
bool ping_sent = false;

static void finish(int result) {
  printf(result == 0 ? "TCP ECHO PASS\n" : "TCP ECHO FAIL\n");
  if (client_fd >= 0) close(client_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
  // The socket is closed and the main loop cancelled, so node's event loop
  // drains and the process exits naturally with status 0. On failure abort()
  // to surface a non-zero exit code to the test harness.
  if (result != 0) abort();
#else
  exit(result);
#endif
}

static void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(client_fd, &fdr);
  FD_SET(client_fd, &fdw);
  select(64, &fdr, &fdw, NULL, &tv);

  // connect completion
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
    printf("connected\n");

    // getpeername goes through emscripten's own address layer, reading the
    // backend's sock fields. Check it reports the server we connected to.
    struct sockaddr_in pa;
    socklen_t pl = sizeof(pa);
    assert(getpeername(client_fd, (struct sockaddr*)&pa, &pl) == 0);
    assert(pa.sin_port == dest.sin_port);
    assert(pa.sin_addr.s_addr == dest.sin_addr.s_addr);
  }

  // send ping
  if (connected && !ping_sent && FD_ISSET(client_fd, &fdw)) {
    if (send(client_fd, "ping", 4, 0) == 4) ping_sent = true;
  }

  // receive the echoed ping
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
  assert(argc > 1 && "usage: test_tcp_echo <port>");
  int port = atoi(argv[1]);

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  fcntl(client_fd, F_SETFL, O_NONBLOCK);

  // Exercise the setsockopt/getsockopt path and check a round-trip.
  int one = 1;
  assert(setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) == 0);
  assert(setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)) == 0);
  int got = 0;
  socklen_t gl = sizeof(got);
  // POSIX only promises a nonzero value for a set boolean option, not exactly 1
  // (macOS reports the internal flag bit, for example).
  assert(getsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &got, &gl) == 0 && got != 0);

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);
  printf("connecting to 127.0.0.1:%d\n", port);

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
