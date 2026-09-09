/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * getaddrinfo() under -sNODERAWSOCKETS: numeric addresses and /etc/hosts
 * entries (read through emscripten's FS) resolve synchronously, a name with
 * several addresses comes back as a linked list, and any other hostname goes
 * to node:dns. That lookup is asynchronous, so it blocks where the calling
 * stack can wait (a proxied pthread, JSPI) and is EAI_AGAIN where it cannot
 * (built with -DNO_WAIT).
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

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

int main(void) {
  mkdir("/etc", 0777);
  FILE* f = fopen("/etc/hosts", "w");
  assert(f);
  fputs("# test hosts\n"
        "10.1.2.3   statichost.test alias.test\n"
        "192.0.2.1  multi.test\n"
        "fe80::1    multi.test\n"
        "192.0.2.2  multi.test # trailing comment\n",
        f);
  fclose(f);

  struct addrinfo* res = lookup("10.9.8.7", AF_UNSPEC, 0);
  assert(count_v4(res, "10.9.8.7") == 1 && !res->ai_next);
  freeaddrinfo(res);

  res = lookup("alias.test", AF_INET, 0);
  assert(count_v4(res, "10.1.2.3") == 1 && !res->ai_next);
  freeaddrinfo(res);

  // Only the matching family, still as a list.
  res = lookup("multi.test", AF_INET, 0);
  assert(count_v4(res, "192.0.2.1") == 1);
  assert(count_v4(res, "192.0.2.2") == 1);
  assert(res->ai_next && !res->ai_next->ai_next);
  freeaddrinfo(res);

  // AF_UNSPEC includes the IPv6 entry.
  res = lookup("multi.test", AF_UNSPEC, 0);
  int n = 0, v6 = 0;
  for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
    n++;
    if (ai->ai_family == AF_INET6) {
      struct sockaddr_in6* sin6 = (struct sockaddr_in6*)ai->ai_addr;
      assert(ai->ai_addrlen == sizeof(*sin6));
      assert(sin6->sin6_addr.s6_addr[0] == 0xfe && sin6->sin6_addr.s6_addr[15] == 1);
      v6++;
    }
  }
  assert(n == 3 && v6 == 1);
  freeaddrinfo(res);

  // Not in /etc/hosts: a real node:dns lookup.
#ifdef NO_WAIT
  lookup("localhost", AF_INET, EAI_AGAIN);
#else
  res = lookup("localhost", AF_INET, 0);
  assert(count_v4(res, "127.0.0.1") == 1);
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
