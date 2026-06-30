/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * EPOLLRDHUP over a real TCP connection: a self-contained loopback client/server
 * (blocking, proxied to a worker) establishes a connection, the server
 * half-closes its write side (shutdown(SHUT_WR) -> FIN), and a blocking
 * epoll_wait on the client reports EPOLLRDHUP - the peer read-side hangup,
 * distinct from a full EPOLLHUP. Also checks that EPOLLRDHUP is request-gated:
 * a registration that didn't ask for it does not receive it.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(listen_fd, 4) == 0);
  socklen_t l = sizeof(addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &l) == 0);

  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);

  // The server-side 'connection' can land just after connect() returns, so wait
  // for the listener to be readable before accepting.
  struct pollfd lp = { .fd = listen_fd, .events = POLLIN };
  assert(poll(&lp, 1, -1) == 1 && (lp.revents & POLLIN));
  int peer_fd = accept(listen_fd, NULL, NULL);
  assert(peer_fd >= 0);

  // Server half-closes its write side: the client's read side hangs up (FIN).
  assert(shutdown(peer_fd, SHUT_WR) == 0);

  int ep = epoll_create1(0);
  struct epoll_event ev = { .events = EPOLLIN | EPOLLRDHUP };
  ev.data.fd = client_fd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, client_fd, &ev) == 0);

  struct epoll_event out[4];
  int n = epoll_wait(ep, out, 4, -1); // blocks until the FIN arrives
  assert(n == 1);
  assert(out[0].data.fd == client_fd);
  assert(out[0].events & EPOLLRDHUP); // peer read-side hangup reported
  assert(!(out[0].events & EPOLLHUP)); // not a full hangup: still half-open

  // EPOLLRDHUP is request-gated: a registration that didn't ask for it doesn't
  // get it, even though the read side is hung up (it still reports EPOLLIN).
  int ep2 = epoll_create1(0);
  ev.events = EPOLLIN;
  assert(epoll_ctl(ep2, EPOLL_CTL_ADD, client_fd, &ev) == 0);
  assert(epoll_wait(ep2, out, 4, 0) == 1);
  assert(out[0].events & EPOLLIN);
  assert(!(out[0].events & EPOLLRDHUP));

  close(ep);
  close(ep2);
  close(client_fd);
  close(peer_fd);
  close(listen_fd);
  printf("EPOLL RDHUP PASS\n");
  return 0;
}
