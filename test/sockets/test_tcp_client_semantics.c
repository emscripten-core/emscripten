/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Outgoing TCP client error/state semantics against a loopback echo server
 * started by the test harness (port in argv[1]). Checks connecting twice gives
 * EISCONN, that shutdown(SHUT_WR) half-closes the write side while reads still
 * work, that writing after that gives EPIPE, and that a full
 * shutdown(SHUT_RDWR) hangs up both halves (POLLHUP). Plain POSIX, also runs
 * natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
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

int fd = -1;
struct sockaddr_in dest;
bool connected = false;
bool ping_sent = false;
bool echoed = false;

void test_success(void) {
  printf("done\n");
  if (fd >= 0) close(fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(fd, &fdr);
  FD_SET(fd, &fdw);
  select(64, &fdr, &fdw, NULL, &tv);

  if (!connected && FD_ISSET(fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &l);
    assert(err == 0 && "connect failed");
    connected = true;

    // Connecting an already-connected socket must report EISCONN.
    int r = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
    assert(r == -1 && errno == EISCONN);
  }

  if (connected && !ping_sent && FD_ISSET(fd, &fdw)) {
    if (send(fd, "ping", 4, 0) == 4) ping_sent = true;
  }

  if (ping_sent && !echoed && FD_ISSET(fd, &fdr)) {
    char buf[4];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    assert(n == 4 && memcmp(buf, "ping", 4) == 0 && "unexpected echo");
    echoed = true;

    // Half-close the write side. The read side must still be usable, so this
    // returns 0 rather than tearing the socket down.
    assert(shutdown(fd, SHUT_WR) == 0);

    // Writing after a write-shutdown is a broken pipe.
    ssize_t w = send(fd, "more", 4, 0);
    assert(w == -1 && errno == EPIPE);

    // A full local shutdown hangs up both halves: poll must report POLLHUP,
    // matching Linux epoll even though the peer hasn't closed.
    assert(shutdown(fd, SHUT_RDWR) == 0);
    struct pollfd pfd = {.fd = fd, .events = POLLIN | POLLOUT};
    assert(poll(&pfd, 1, 1000) > 0);
    assert(pfd.revents & POLLHUP);

    test_success();
  }
}

int main(int argc, char** argv) {
  assert(argc > 1 && "usage: test_tcp_client_semantics <port>");
  signal(SIGPIPE, SIG_IGN); // so the EPIPE write does not kill us natively

  fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(atoi(argv[1]));
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

  int r = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
  assert((r == 0 || errno == EINPROGRESS) && "connect");

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
