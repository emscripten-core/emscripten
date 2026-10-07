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
 * (built with -DNO_WAIT). gethostbyname() resolves through the same path, and
 * getnameinfo()/gethostbyaddr() are node:dns reverse lookups.
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

struct addrinfo* lookup(const char* name, int family, int expect) {
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

int count_v4(struct addrinfo* res, const char* addr) {
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

int reverse(const char* addr, int flags, char* host, size_t hostlen) {
  struct sockaddr_in sa = {0};
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(addr);
  return getnameinfo(
    (struct sockaddr*)&sa, sizeof(sa), host, hostlen, NULL, 0, flags);
}

int main(void) {
  char host[256];
  struct addrinfo* res = lookup("10.9.8.7", AF_UNSPEC, 0);
  assert(count_v4(res, "10.9.8.7") == 1 && !res->ai_next);
  freeaddrinfo(res);

  // gethostbyname() of a numeric address needs no lookup either.
  struct hostent* h = gethostbyname("10.9.8.7");
  assert(h && h->h_addrtype == AF_INET && h->h_length == 4);
  assert(((struct in_addr*)h->h_addr_list[0])->s_addr == inet_addr("10.9.8.7"));
  assert(!h->h_addr_list[1]);
  assert(!strcmp(h->h_name, "10.9.8.7"));

  // A hostname is a real node:dns lookup.
#ifdef NO_WAIT
  lookup("localhost", AF_INET, EAI_AGAIN);
  assert(!gethostbyname("localhost") && h_errno == TRY_AGAIN);
  // A reverse lookup that cannot wait is numeric, or EAI_AGAIN if a name is
  // required.
  assert(reverse("127.0.0.1", 0, host, sizeof(host)) == 0);
  assert(!strcmp(host, "127.0.0.1"));
  assert(reverse("127.0.0.1", NI_NAMEREQD, host, sizeof(host)) == EAI_AGAIN);
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

  // gethostbyname() resolves through the same lookup.
  h = gethostbyname("localhost");
  assert(h && h->h_addrtype == AF_INET && h->h_length == 4);
  assert(!strcmp(h->h_name, "localhost"));
  int found = 0;
  for (char** a = h->h_addr_list; *a; a++) {
    if (((struct in_addr*)*a)->s_addr == inet_addr("127.0.0.1"))
      found++;
  }
  assert(found == 1);
  assert(!gethostbyname("nonexistent.invalid"));
  assert(h_errno == HOST_NOT_FOUND || h_errno == TRY_AGAIN);

  // Reverse lookups through getnameinfo() and gethostbyaddr(). The loopback
  // name is whatever the host's resolver says (localhost, or e.g. the
  // machine's own name), but it is a name.
  assert(reverse("127.0.0.1", NI_NAMEREQD, host, sizeof(host)) == 0);
  printf("getnameinfo(127.0.0.1) -> %s\n", host);
  assert(strcmp(host, "127.0.0.1"));
  struct in_addr loopback = {.s_addr = inet_addr("127.0.0.1")};
  h = gethostbyaddr(&loopback, sizeof(loopback), AF_INET);
  assert(h && !strcmp(h->h_name, host));
  assert(reverse("127.0.0.1", NI_NUMERICHOST, host, sizeof(host)) == 0);
  assert(!strcmp(host, "127.0.0.1"));
#endif

  printf("done\n");
  return 0;
}
