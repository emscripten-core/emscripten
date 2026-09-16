/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * O_NONBLOCK follows Linux: SOCK_NONBLOCK on socket() and accept4() sets it, an
 * accepted fd never inherits it from the listener, and a non-blocking connect
 * reports EINPROGRESS then POLLOUT with SO_ERROR 0.
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int is_nonblock(int fd) {
  int fl = fcntl(fd, F_GETFL);
  assert(fl != -1);
  return !!(fl & O_NONBLOCK);
}

void wait_for(int fd, short events) {
  struct pollfd p = { .fd = fd, .events = events };
  assert(poll(&p, 1, -1) == 1);
  assert(p.revents & events);
}

int make_listener(int type, struct sockaddr_in* addr) {
  int fd = socket(AF_INET, type, 0);
  assert(fd >= 0);
  memset(addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);
  assert(bind(fd, (struct sockaddr*)addr, sizeof(*addr)) == 0);
  assert(listen(fd, 4) == 0);
  socklen_t l = sizeof(*addr);
  assert(getsockname(fd, (struct sockaddr*)addr, &l) == 0);
  return fd;
}

void check_accept(int listen_fd, struct sockaddr_in* addr, int flags, int expect_nonblock) {
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)addr, sizeof(*addr)) == 0);
  wait_for(listen_fd, POLLIN);
  int peer_fd = flags ? accept4(listen_fd, NULL, NULL, flags) : accept(listen_fd, NULL, NULL);
  assert(peer_fd >= 0);
  assert(is_nonblock(peer_fd) == expect_nonblock);
  close(peer_fd);
  close(client_fd);
}

int main(void) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0 && !is_nonblock(fd));
  close(fd);
  fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  assert(fd >= 0 && is_nonblock(fd));
  close(fd);
  fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
  assert(fd >= 0 && is_nonblock(fd));
  close(fd);

  struct sockaddr_in blocking_addr, nonblocking_addr;
  int blocking_fd = make_listener(SOCK_STREAM, &blocking_addr);
  int nonblocking_fd = make_listener(SOCK_STREAM | SOCK_NONBLOCK, &nonblocking_addr);
  assert(!is_nonblock(blocking_fd));
  assert(is_nonblock(nonblocking_fd));

  assert(accept(nonblocking_fd, NULL, NULL) == -1 && errno == EAGAIN);
#ifdef __EMSCRIPTEN__
  // A blocking accept cannot block, so it would-blocks too.
  assert(accept(blocking_fd, NULL, NULL) == -1 && errno == EAGAIN);
#endif

  check_accept(blocking_fd, &blocking_addr, 0, 0);
  check_accept(blocking_fd, &blocking_addr, SOCK_NONBLOCK | SOCK_CLOEXEC, 1);
  check_accept(nonblocking_fd, &nonblocking_addr, 0, 0);
  check_accept(nonblocking_fd, &nonblocking_addr, SOCK_NONBLOCK | SOCK_CLOEXEC, 1);

  int client_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&blocking_addr, sizeof(blocking_addr)) == -1);
  assert(errno == EINPROGRESS);
  wait_for(client_fd, POLLOUT);
  int err = -1;
  socklen_t l = sizeof(err);
  assert(getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &err, &l) == 0);
  assert(err == 0);
  close(client_fd);

  close(blocking_fd);
  close(nonblocking_fd);
  printf("done\n");
  return 0;
}
