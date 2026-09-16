/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * getaddrinfo() under -sNODERAWSOCKETS: numeric addresses resolve
 * synchronously, and any hostname goes to node:dns, returning every address as
 * a linked list. That lookup is asynchronous, so it blocks where the calling
 * stack can wait (a proxied pthread, JSPI) and is EAI_AGAIN where it cannot
 * (built with -DNO_WAIT).
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten/eventloop.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static struct addrinfo* lookup(const char* name, int family, int expect) {
  struct addrinfo hints = {0};
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo* res = NULL;
  int err = getaddrinfo(name, "80", &hints, &res);
  if (err != expect) {
    printf("getaddrinfo(%s) = %d, expected %d\n", name, err, expect);
    exit(1);
  }
  return res;
}

static int count_v4(struct addrinfo* res, const char* addr) {
  int n = 0;
  for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
    assert(ai->ai_socktype == SOCK_STREAM);
    assert(ai->ai_protocol == IPPROTO_TCP);
    if (ai->ai_family != AF_INET) continue;
    struct sockaddr_in* sin = (struct sockaddr_in*)ai->ai_addr;
    assert(ai->ai_addrlen == sizeof(*sin));
    assert(ntohs(sin->sin_port) == 80);
    if (sin->sin_addr.s_addr == inet_addr(addr)) n++;
  }
  return n;
}

int ticked = 0;
void tick(void* arg) { ticked = 1; }

int main(void) {
  struct addrinfo* res = lookup("10.9.8.7", AF_UNSPEC, 0);
  assert(count_v4(res, "10.9.8.7") == 1 && !res->ai_next);
  freeaddrinfo(res);

  // A hostname is a real node:dns lookup.
#ifdef NO_WAIT
  lookup("localhost", AF_INET, EAI_AGAIN);
#else
  // A user callback completing while main() is suspended in the lookup must
  // not exit the runtime (EXIT_RUNTIME). Under PROXY_TO_PTHREAD the calling
  // thread is parked, so the timer only runs once the lookup has returned.
  emscripten_set_timeout(tick, 0, NULL);
  res = lookup("localhost", AF_INET, 0);
#ifndef __EMSCRIPTEN_PTHREADS__
  assert(ticked);
#endif
  assert(count_v4(res, "127.0.0.1") == 1);
  for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
    assert(ai->ai_family == AF_INET);
  }
  freeaddrinfo(res);

  // AF_UNSPEC returns every address the resolver has, each in its own family.
  res = lookup("localhost", AF_UNSPEC, 0);
  assert(count_v4(res, "127.0.0.1") == 1);
  for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
    if (ai->ai_family == AF_INET6) {
      struct sockaddr_in6* sin6 = (struct sockaddr_in6*)ai->ai_addr;
      assert(ai->ai_addrlen == sizeof(*sin6));
      assert(ntohs(sin6->sin6_port) == 80);
      assert(IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr));
    } else {
      assert(ai->ai_family == AF_INET);
    }
  }
  freeaddrinfo(res);

  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  res = NULL;
  int err = getaddrinfo("nonexistent.invalid", NULL, &hints, &res);
  assert(err == EAI_NONAME || err == EAI_AGAIN);
  assert(!res);
#endif

  printf("done\n");
  return 0;
}
