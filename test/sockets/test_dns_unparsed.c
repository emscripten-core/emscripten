/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * getaddrinfo() under -sNODERAWSOCKETS skips resolver results that are not
 * numeric addresses. A --pre-js stubs node:dns.lookup so that mixed.test
 * returns a CNAME target name alongside one real IPv4 address, and names.test
 * returns only names.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main(void) {
  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo* res = NULL;

  // Only the parsable entry is returned.
  int err = getaddrinfo("mixed.test", "443", &hints, &res);
  printf("getaddrinfo(mixed.test) = %d\n", err);
  assert(err == 0 && res);
  assert(res->ai_family == AF_INET && !res->ai_next);
  struct sockaddr_in* sin = (struct sockaddr_in*)res->ai_addr;
  assert(sin->sin_addr.s_addr == inet_addr("20.250.119.67"));
  assert(ntohs(sin->sin_port) == 443);
  freeaddrinfo(res);

  // With nothing parsable left, the name has no address.
  res = NULL;
  err = getaddrinfo("names.test", "443", &hints, &res);
  printf("getaddrinfo(names.test) = %d\n", err);
  assert(err == EAI_NONAME && !res);

  printf("done\n");
  return 0;
}
