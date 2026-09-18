/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_dns_lookup_async(): a getaddrinfo() that never blocks. It returns
 * an fd that is readable once the lookup completes, so the result can be waited
 * for with poll() from any stack, including the main thread of a plain build
 * where getaddrinfo() itself cannot wait. Numeric addresses and errors are
 * readable at once; a hostname completes after a real node:dns lookup (or
 * immediately with a fake address without -sNODERAWSOCKETS).
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int fd;
int polls;

int start(const char* name, int family, const char* service) {
  struct addrinfo hints = {0};
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  int fd = emscripten_dns_lookup_async(name, service, &hints);
  assert(fd >= 0);
  return fd;
}

int readable(int fd) {
  struct pollfd p = { .fd = fd, .events = POLLIN };
  int n = poll(&p, 1, 0);
  assert(n == 0 || (n == 1 && (p.revents & POLLIN)));
  return n;
}

void check_v4(struct addrinfo* res, const char* addr) {
  int n = 0;
  for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
    assert(ai->ai_socktype == SOCK_STREAM);
    assert(ai->ai_protocol == IPPROTO_TCP);
    assert(ai->ai_family == AF_INET);
    struct sockaddr_in* sin = (struct sockaddr_in*)ai->ai_addr;
    assert(ai->ai_addrlen == sizeof(*sin));
    assert(ntohs(sin->sin_port) == 80);
    if (!addr || sin->sin_addr.s_addr == inet_addr(addr)) n++;
  }
  assert(n >= 1);
}

void finish(void* arg) {
  if (!readable(fd)) {
    polls++;
#ifdef __EMSCRIPTEN_PTHREADS__
    // A pthread can simply block on the fd.
    struct pollfd p = { .fd = fd, .events = POLLIN };
    assert(poll(&p, 1, -1) == 1 && (p.revents & POLLIN));
#else
    // The main thread cannot: retry on the next turn of the event loop.
    emscripten_set_timeout(finish, 0, NULL);
    return;
#endif
  }
  struct addrinfo* res = NULL;
  assert(emscripten_dns_lookup_result(fd, &res) == 0);
#ifdef REAL_DNS
#ifndef __EMSCRIPTEN_PTHREADS__
  assert(polls > 0);
#endif
  check_v4(res, "127.0.0.1");
#else
  check_v4(res, NULL);
#endif
  freeaddrinfo(res);
  assert(close(fd) == 0);
  printf("done\n");
}

int main(void) {
  // A numeric address needs no lookup: readable on return.
  fd = start("10.9.8.7", AF_UNSPEC, "80");
  assert(readable(fd));
  struct addrinfo* res = NULL;
  assert(emscripten_dns_lookup_result(fd, &res) == 0);
  check_v4(res, "10.9.8.7");
  assert(!res->ai_next);
  freeaddrinfo(res);
  assert(close(fd) == 0);

  // So does an error.
  fd = start("10.9.8.7", AF_UNSPEC, "http");
  assert(readable(fd));
  assert(emscripten_dns_lookup_result(fd, &res) == EAI_SERVICE);
  assert(close(fd) == 0);

  // Nothing is allocated until a result is read: each read mints its own list,
  // and closing without reading has nothing to free.
  fd = start("10.9.8.7", AF_UNSPEC, "80");
  struct addrinfo* res2 = NULL;
  assert(emscripten_dns_lookup_result(fd, &res) == 0);
  assert(emscripten_dns_lookup_result(fd, &res2) == 0);
  assert(res != res2 && res->ai_addr != res2->ai_addr);
  freeaddrinfo(res);
  freeaddrinfo(res2);
  assert(close(fd) == 0);
  fd = start("10.9.8.7", AF_UNSPEC, "80");
  assert(close(fd) == 0);

  // Not a lookup fd.
  assert(emscripten_dns_lookup_result(0, &res) == EAI_SYSTEM);

  // dup() shares the one lookup; the result is readable through either fd.
  fd = start("10.9.8.7", AF_UNSPEC, "80");
  int d = dup(fd);
  assert(d >= 0);
  assert(close(fd) == 0);
  assert(readable(d));
  assert(emscripten_dns_lookup_result(d, &res) == 0);
  check_v4(res, "10.9.8.7");
  freeaddrinfo(res);
  assert(close(d) == 0);

  // Closing while a real lookup is still pending is fine: its result is just
  // dropped.
  fd = start("localhost", AF_INET, "80");
  assert(close(fd) == 0);

  // A hostname: pending until node:dns answers, EAI_AGAIN until then. (On a
  // pthread the proxied calls give the main thread's loop turns in between, so
  // it may already have completed.)
  fd = start("localhost", AF_INET, "80");
#if defined(REAL_DNS) && !defined(__EMSCRIPTEN_PTHREADS__)
  assert(!readable(fd));
  assert(emscripten_dns_lookup_result(fd, &res) == EAI_AGAIN);
#endif
  finish(NULL);
  return 0;
}
