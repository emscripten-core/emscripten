/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * O_NONBLOCK follows Linux: SOCK_NONBLOCK on socket() and accept4() sets it,
 * FIONBIO toggles it, an accepted fd never inherits it from the listener, and a
 * non-blocking connect (TCP and AF_UNIX) reports EINPROGRESS then POLLOUT with
 * SO_ERROR 0.
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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
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

void check_connect(int fd, struct sockaddr* addr, socklen_t len) {
  int r = connect(fd, addr, len);
#ifdef __EMSCRIPTEN__
  assert(r == -1 && errno == EINPROGRESS);
#else
  assert(r == 0 || errno == EINPROGRESS);
#endif
  wait_for(fd, POLLOUT);
  int err = -1;
  socklen_t l = sizeof(err);
  assert(getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &l) == 0);
  assert(err == 0);
}

void check_accept(int listen_fd, struct sockaddr* addr, socklen_t len, int flags, int expect_nonblock) {
  int client_fd = socket(addr->sa_family, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, addr, len) == 0);
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

  fd = socket(AF_INET, SOCK_STREAM, 0);
  int on = 1;
  assert(ioctl(fd, FIONBIO, &on) == 0 && is_nonblock(fd));
  on = 0;
  assert(ioctl(fd, FIONBIO, &on) == 0 && !is_nonblock(fd));
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

  check_accept(blocking_fd, (struct sockaddr*)&blocking_addr, sizeof(blocking_addr), 0, 0);
  check_accept(blocking_fd, (struct sockaddr*)&blocking_addr, sizeof(blocking_addr), SOCK_NONBLOCK | SOCK_CLOEXEC, 1);
  check_accept(nonblocking_fd, (struct sockaddr*)&nonblocking_addr, sizeof(nonblocking_addr), 0, 0);
  check_accept(nonblocking_fd, (struct sockaddr*)&nonblocking_addr, sizeof(nonblocking_addr), SOCK_NONBLOCK | SOCK_CLOEXEC, 1);

  int client_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(client_fd >= 0);
  check_connect(client_fd, (struct sockaddr*)&blocking_addr, sizeof(blocking_addr));
  close(client_fd);

  struct sockaddr_un un;
  memset(&un, 0, sizeof(un));
  un.sun_family = AF_UNIX;
  strcpy(un.sun_path, "nonblock_flags.sock");
  unlink(un.sun_path);
  int unix_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  assert(unix_fd >= 0);
  assert(bind(unix_fd, (struct sockaddr*)&un, sizeof(un)) == 0);
  assert(listen(unix_fd, 4) == 0);
  client_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(client_fd >= 0);
  check_connect(client_fd, (struct sockaddr*)&un, sizeof(un));
  close(client_fd);
  check_accept(unix_fd, (struct sockaddr*)&un, sizeof(un), 0, 0);
  check_accept(unix_fd, (struct sockaddr*)&un, sizeof(un), SOCK_NONBLOCK, 1);
  close(unix_fd);
  unlink(un.sun_path);

  close(blocking_fd);
  close(nonblocking_fd);
  printf("done\n");
  return 0;
}
