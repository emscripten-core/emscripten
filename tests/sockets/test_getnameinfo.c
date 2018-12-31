#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int main() {
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct sockaddr_in sa4;
  struct sockaddr_in6 sa6;
  char node[256];
  char serv[256];
  int flags;
  int err;

#ifndef __APPLE__
  // incorrect sockaddr size
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = AF_INET;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4)-1, NULL, 0, NULL, 0, 0);
  assert(err == EAI_FAMILY);

  memset(&sa6, 0, sizeof(sa6));
  sa6.sin6_family = AF_INET6;
  err = getnameinfo((struct sockaddr*)&sa6, sizeof(sa6)-1, NULL, 0, NULL, 0, 0);
  assert(err == EAI_FAMILY);

  // invalid family
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = 9999;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), NULL, 0, NULL, 0, 0);
  assert(err == EAI_FAMILY);
#endif

  // NI_NUMERICHOST and NI_NAMEREQD conflict
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = AF_INET;
  flags = NI_NUMERICHOST | NI_NAMEREQD;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), node, sizeof(node), serv, sizeof(serv), flags);
  assert(err == EAI_NONAME);

  // too small of buffer
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = AF_INET;
  *(uint32_t*)&sa4.sin_addr = 67305985;
  sa4.sin_port = htons(54321);
  flags = NI_NUMERICHOST;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), node, 1, serv, sizeof(serv), flags);
  assert(err == EAI_OVERFLOW);
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), node, sizeof(node), serv, 1, flags);
  assert(err == EAI_OVERFLOW);

  // NI_NAMEREQD and lookup failed
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = AF_INET;
  *(uint32_t*)&sa4.sin_addr = 67305985;
  flags = NI_NAMEREQD;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), node, sizeof(node), serv, sizeof(serv), flags);
  assert(err == EAI_NONAME);

  // lookup failed
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = AF_INET;
  *(uint32_t*)&sa4.sin_addr = 67305985;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), node, sizeof(node), serv, sizeof(serv), flags);
  assert(err == EAI_NONAME);

  // no lookup
  memset(&sa4, 0, sizeof(sa4));
  sa4.sin_family = AF_INET;
  *(uint32_t*)&sa4.sin_addr = 67305985;
  sa4.sin_port = htons(54321);
  flags = NI_NUMERICHOST;
  err = getnameinfo((struct sockaddr*)&sa4, sizeof(sa4), node, sizeof(node), serv, sizeof(serv), flags);
  assert(!err);
  assert(!strcmp(node, "1.2.3.4"));
  assert(!strcmp(serv, "54321"));

  // lookup
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo("www.mozilla.org", "54321", &hints, &servinfo);
  assert(!err);
  flags = NI_NAMEREQD;
  err = getnameinfo(servinfo->ai_addr, servinfo->ai_addrlen, node, sizeof(node), serv, sizeof(serv), flags);
  assert(!err);
  assert(strstr(node, "mozilla"));
  assert(!strcmp(serv, "54321"));

  puts("success");

  return EXIT_SUCCESS;
}
