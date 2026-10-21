/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * gethostbyname(), getaddrinfo() and getnameinfo() share the fake DNS table
 * of the socket emulation: a name maps to one synthetic address whichever
 * function resolves it, and the address maps back to that name.  With
 * -sPROXY_TO_PTHREAD all of them must use the main thread's table, where
 * connect() looks the address up to build the WebSocket URL.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static struct in_addr resolve_getaddrinfo(const char* name) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  struct addrinfo* res = NULL;
  int err = getaddrinfo(name, NULL, &hints, &res);
  assert(err == 0);
  struct in_addr addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
  freeaddrinfo(res);
  return addr;
}

static void check_reverse(struct in_addr addr, const char* expected) {
  struct sockaddr_in sa = {0};
  sa.sin_family = AF_INET;
  sa.sin_addr = addr;
  char host[256];
  int err = getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host),
                        NULL, 0, NI_NAMEREQD);
  printf("getnameinfo(%s) -> %s\n", inet_ntoa(addr), err ? gai_strerror(err) : host);
  assert(err == 0);
  assert(strcmp(host, expected) == 0);
}

int main() {
  // A first entry, created by getaddrinfo.
  struct in_addr first = resolve_getaddrinfo("first.example.com");

  // A second name, resolved with gethostbyname: getaddrinfo must agree.
  struct hostent* h = gethostbyname("second.example.com");
  assert(h && h->h_addrtype == AF_INET);
  struct in_addr second = *(struct in_addr*)h->h_addr_list[0];
  struct in_addr again = resolve_getaddrinfo("second.example.com");
  printf("second.example.com: gethostbyname -> %s, ", inet_ntoa(second));
  printf("getaddrinfo -> %s\n", inet_ntoa(again));
  assert(second.s_addr == again.s_addr);
  assert(second.s_addr != first.s_addr);

  // Both addresses map back to their names.
  check_reverse(first, "first.example.com");
  check_reverse(second, "second.example.com");

  puts("done");
  return 0;
}
