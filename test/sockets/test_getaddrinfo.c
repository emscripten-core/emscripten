/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE

#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

void print_sockaddr(struct sockaddr_in *sa4, int socktype, int counter) {
  printf("addr[%d] v4: (socktype=%d family=%d) %s:%d\n", counter, socktype, sa4->sin_family, inet_ntoa(sa4->sin_addr), ntohs(sa4->sin_port));
}

void print_sockaddr6(struct sockaddr_in6 *sa6, int socktype, int counter) {
  char buf[1024];
  printf("addr[%d] v6: (socktype=%d family=%d) %s:%d\n", counter, socktype, sa6->sin6_family, inet_ntop(sa6->sin6_family, &sa6->sin6_addr, buf, sizeof(buf)), ntohs(sa6->sin6_port));
}

void print_addrinfo_next(struct addrinfo *info, int counter) {
  if (info->ai_family == AF_INET) {
    print_sockaddr(((struct sockaddr_in*)info->ai_addr), info->ai_socktype, counter);
  } else if (info->ai_family == AF_INET6) {
    print_sockaddr6(((struct sockaddr_in6*)info->ai_addr), info->ai_socktype, counter);
  } else {
    assert(false);
  }
  if (info->ai_next) {
    print_addrinfo_next(info->ai_next, counter+1);
  }
}

void print_addrinfo(struct addrinfo *info) {
  print_addrinfo_next(info, 0);
}

#define CHECK_ERR(code, expected)                                              \
  do {                                                                         \
    const char* err = gai_strerror(code);                                      \
    printf("%s -> %s\n", #code, err);                                          \
    assert(strcmp(err, expected) == 0);                                        \
  } while (0)

int main() {
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct sockaddr_in *sa4;
  struct sockaddr_in6 *sa6;
  int err;

  char buf[1024];
  printf("in6addr_loopback: %s\n", inet_ntop(AF_INET6, &in6addr_loopback, buf, sizeof(buf)));

  // no name or service
  //err = getaddrinfo(NULL, NULL, NULL, &servinfo);

  // XXX musl follows the spec precisely, and it does not allow both to be NULL,
  // despite documenting EAI_NONAME as the right result for that case
  // assert(err == EAI_NONAME);

  // invalid socket type
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = 9999;
  err = getaddrinfo("www.mozilla.org", "80", &hints, &servinfo);
#ifdef __APPLE__
  assert(err == EAI_BADHINTS);
#else
  assert(err == EAI_SOCKTYPE);
#endif

  // invalid family
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNIX;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("www.mozilla.org", "80", &hints, &servinfo);
  assert(err == EAI_FAMILY);

  // invalid service
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("www.mozilla.org", "foobar", &hints, &servinfo);
#ifdef __APPLE__
  assert(err == EAI_NONAME);
#else
  assert(err == EAI_SERVICE);
#endif

  // test loopback resolution (ipv4)
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo(NULL, "80", &hints, &servinfo);
  assert(!err);
  print_addrinfo(servinfo);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(*(uint32_t*)&(sa4->sin_addr) == ntohl(INADDR_LOOPBACK));
  assert(sa4->sin_port == ntohs(80));
  freeaddrinfo(servinfo);

  // test loopback resolution (ipv6)
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo(NULL, "81", &hints, &servinfo);
  assert(!err);
  sa6 = ((struct sockaddr_in6*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET6);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(memcmp(&sa6->sin6_addr, &in6addr_loopback, sizeof(in6addr_loopback)) == 0);
  assert(sa6->sin6_port == ntohs(81));
  freeaddrinfo(servinfo);

  // test bind preparation
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo(NULL, "82", &hints, &servinfo);
  assert(!err);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(*(uint32_t*)&(sa4->sin_addr) == 0);
  assert(sa4->sin_port == ntohs(82));
  freeaddrinfo(servinfo);

  // test numeric address (ipv4)
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  err = getaddrinfo("1.2.3.4", "83", &hints, &servinfo);
  assert(!err);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_DGRAM);
  assert(*(uint32_t*)&(sa4->sin_addr) == 67305985);
  assert(sa4->sin_port == ntohs(83));
  freeaddrinfo(servinfo);

  // test numeric address (ipv4 address specified as ipv6 with AI_V4MAPPED)
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_V4MAPPED;
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("1.2.3.4", "84", &hints, &servinfo);
  assert(!err);
  sa6 = ((struct sockaddr_in6*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET6);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(*((uint32_t*)&(sa6->sin6_addr)+2) == htonl(0xffff));
  assert(*((uint32_t*)&(sa6->sin6_addr)+3) == 67305985);
  assert(sa6->sin6_port == ntohs(84));
  freeaddrinfo(servinfo);

  // test numeric address (ipv4 address specified as ipv6 without AI_V4MAPPED)
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("1.2.3.4", "85", &hints, &servinfo);
#ifdef __linux__
  assert(err == -9 /* EAI_ADDRFAMILY */);
#else
  assert(err == EAI_NONAME);
#endif

  // test numeric address (ipv6)
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  err = getaddrinfo("2001:0db8:85a3:0042:1000:8a2e:0370:7334", "86", &hints, &servinfo);
  assert(!err);
  sa6 = ((struct sockaddr_in6*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET6);
  assert(servinfo->ai_socktype == SOCK_DGRAM);
  assert(*((uint32_t*)&(sa6->sin6_addr)+0) == -1207107296);
  assert(*((uint32_t*)&(sa6->sin6_addr)+1) == 1107338117);
  assert(*((uint32_t*)&(sa6->sin6_addr)+2) == 780795920);
  assert(*((uint32_t*)&(sa6->sin6_addr)+3) == 879980547);
  assert(sa6->sin6_port == ntohs(86));
  freeaddrinfo(servinfo);

  // test numeric address (ipv6 address specified as ipv4)
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("2001:0db8:85a3:0042:1000:8a2e:0370:7334", "87", &hints, &servinfo);
#ifdef __linux__
  assert(err == -9 /* EAI_ADDRFAMILY */);
#else
  assert(err == EAI_NONAME);
#endif

  // test non-numeric host with AI_NUMERICHOST
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_NUMERICHOST;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("www.mozilla.org", "88", &hints, &servinfo);
  assert(err == EAI_NONAME);

  // test non-numeric host with AF_INET
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("www.mozilla.org", "89", &hints, &servinfo);
  assert(!err);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(sa4->sin_port == ntohs(89));
  freeaddrinfo(servinfo);

  // test non-numeric host with AF_INET6
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("www.google.com", "90", &hints, &servinfo);
  assert(!err);
  sa6 = ((struct sockaddr_in6*)servinfo->ai_addr);
  print_addrinfo(servinfo);
  assert(servinfo->ai_family == AF_INET6);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(*((uint32_t*)&(sa6->sin6_addr)+0) != 0 ||
         *((uint32_t*)&(sa6->sin6_addr)+1) != 0 ||
         *((uint32_t*)&(sa6->sin6_addr)+2) != 0 ||
         *((uint32_t*)&(sa6->sin6_addr)+3) != 0);
  assert(sa6->sin6_port == ntohs(90));
  freeaddrinfo(servinfo);

  // test with NULL hints
  // Specifying hints as NULL is equivalent to setting ai_socktype and ai_protocol to 0;
  // ai_family to AF_UNSPEC; and ai_flags to (AI_V4MAPPED | AI_ADDRCONFIG)
  // N.B. with NULL hints getaddrinfo should really be passing back multiple addrinfo structures in a
  // linked list with next values given in ai_next. The current implementation doesn't do that yet but the
  // following tests have assert(servinfo->ai_next == NULL) so that they will fail when multiple values do
  // eventually get implemented, so we know to improve the tests then to cope with multiple values.

  // test numeric host
  err = getaddrinfo("1.2.3.4", "85", NULL, &servinfo);
  assert(!err);
  print_addrinfo(servinfo);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(servinfo->ai_protocol == IPPROTO_TCP);
  assert(sa4->sin_port == ntohs(85));
  freeaddrinfo(servinfo);

  // test non-numeric host
  err = getaddrinfo("www.mozilla.org", "89", NULL, &servinfo);
  assert(!err);
  print_addrinfo(servinfo);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(servinfo->ai_protocol == IPPROTO_TCP);
  assert(sa4->sin_port == ntohs(89));
  freeaddrinfo(servinfo);

  // test loopback resolution
  hints.ai_family = AF_INET;
  err = getaddrinfo(NULL, "80", &hints, &servinfo);
  assert(!err);
  print_addrinfo(servinfo);
  sa4 = ((struct sockaddr_in*)servinfo->ai_addr);
  assert(servinfo->ai_family == AF_INET);
  assert(servinfo->ai_socktype == SOCK_STREAM);
  assert(servinfo->ai_protocol == IPPROTO_TCP);
  assert(sa4->sin_port == ntohs(80));
  freeaddrinfo(servinfo);

#ifdef __EMSCRIPTEN__
  // test gai_strerror
  CHECK_ERR(0, "Unknown error");
  CHECK_ERR(EAI_BADFLAGS, "Invalid flags");
  CHECK_ERR(EAI_NONAME, "Name does not resolve");
  CHECK_ERR(EAI_AGAIN, "Try again");
  CHECK_ERR(EAI_FAIL, "Non-recoverable error");
  CHECK_ERR(EAI_FAMILY, "Unrecognized address family or invalid length");
  CHECK_ERR(EAI_SOCKTYPE, "Unrecognized socket type");
  CHECK_ERR(EAI_SERVICE, "Unrecognized service");
  CHECK_ERR(EAI_MEMORY, "Out of memory");
  CHECK_ERR(EAI_SYSTEM, "System error");
  CHECK_ERR(EAI_OVERFLOW, "Overflow");
  CHECK_ERR(EAI_NODATA, "Name has no usable address");
  CHECK_ERR(-9, "Unknown error");
  CHECK_ERR(-13, "Unknown error");
  CHECK_ERR(-100, "Unknown error");
#endif

  puts("success");

  return EXIT_SUCCESS;
}

