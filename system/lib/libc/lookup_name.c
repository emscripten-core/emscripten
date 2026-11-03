// Emscripten-specific version of musl/src/network/lookup_name.c: the
// gethostbyname family resolves through getaddrinfo so that both use the
// same resolver (the fake DNS table, node:dns under NODERAWSOCKETS, or the
// socket proxy under PROXY_POSIX_SOCKETS).

#include "musl/src/network/lookup.h"
#include <string.h>

int __lookup_name(struct address buf[static MAXADDRS],
                  char canon[static 256],
                  const char* name,
                  int family,
                  int flags) {
  /* This hunk is duplicated from musl/src/network/lookup_name.c */
  *canon = 0;
  if (name) {
    /* reject empty name and check len so it fits into temp bufs */
    size_t l = strnlen(name, 255);
    if (l - 1 >= 254)
      return EAI_NONAME;
    memcpy(canon, name, l + 1);
  }

  struct addrinfo hints = {.ai_family = family, .ai_socktype = SOCK_STREAM};
  struct addrinfo* res;
  int err = getaddrinfo(name, 0, &hints, &res);
  if (err)
    return err;

  int cnt = 0;
  for (struct addrinfo* ai = res; ai && cnt < MAXADDRS; ai = ai->ai_next) {
    memset(&buf[cnt], 0, sizeof buf[cnt]);
    buf[cnt].family = ai->ai_family;
    if (ai->ai_family == AF_INET6) {
      struct sockaddr_in6* sin6 = (void*)ai->ai_addr;
      memcpy(buf[cnt].addr, &sin6->sin6_addr, 16);
      buf[cnt].scopeid = sin6->sin6_scope_id;
    } else {
      memcpy(buf[cnt].addr, &((struct sockaddr_in*)ai->ai_addr)->sin_addr, 4);
    }
    cnt++;
  }
  freeaddrinfo(res);
  return cnt ? cnt : EAI_NONAME;
}
