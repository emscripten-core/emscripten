/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * The fd returned by emscripten_dns_lookup_async() becomes readable once the
 * lookup completes. It is added to an epoll set and emscripten_epoll_set_callback()
 * awaits it without blocking, driving the lookup purely through that callback
 * with no main loop or blocking poll/select.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

static int ep;

static void fail(const char* why) {
  printf("DNS CALLBACK FAIL: %s\n", why);
  abort();
}

static void on_ready(int epfd, struct epoll_event* events, int nready, void* ud) {
  if (nready != 1 || !(events[0].events & EPOLLIN)) fail("lookup fd did not become readable");
  int fd = events[0].data.fd;

  struct addrinfo* res = NULL;
  int result = emscripten_dns_lookup_result(fd, &res);
  close(fd);
  close(ep);
  if (result != 0) fail("async lookup failed");
  assert(res);
  unsigned addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  if (addr != htonl(INADDR_LOOPBACK)) fail("localhost did not resolve to 127.0.0.1");

  printf("DNS CALLBACK PASS\n");
}

int main(void) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int lookup_fd = emscripten_dns_lookup_async("localhost", NULL, &hints);
  if (lookup_fd < 0) fail("async lookup did not return an fd");

  ep = epoll_create1(0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = lookup_fd;
  if (epoll_ctl(ep, EPOLL_CTL_ADD, lookup_fd, &ev) != 0) fail("epoll_ctl add");

  // The armed callback keeps the runtime alive; on_ready fires when the lookup
  // fd becomes readable. Closing the epoll fd in the callback ends the interest.
  emscripten_epoll_set_callback(ep, 1, on_ready, NULL);
  return 0;
}
