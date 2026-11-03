/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * IP_ADD_MEMBERSHIP / IP_DROP_MEMBERSHIP and their IPv6 equivalents reach the
 * kernel: a duplicate join fails with EADDRINUSE and leaving a group that was
 * not joined fails with EADDRNOTAVAIL. A join before bind() is applied once
 * the socket is bound. The options are set-only, so getsockopt rejects them.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct ip_mreq mreq4(const char* group) {
  struct ip_mreq m;
  memset(&m, 0, sizeof(m));
  inet_pton(AF_INET, group, &m.imr_multiaddr);
  inet_pton(AF_INET, "127.0.0.1", &m.imr_interface);
  return m;
}

int set4(int fd, int opt, const char* group) {
  struct ip_mreq m = mreq4(group);
  return setsockopt(fd, IPPROTO_IP, opt, &m, sizeof(m)) == 0 ? 0 : errno;
}

int set6(int fd, int opt, const char* group) {
  struct ipv6_mreq m;
  memset(&m, 0, sizeof(m));
  inet_pton(AF_INET6, group, &m.ipv6mr_multiaddr);
  return setsockopt(fd, IPPROTO_IPV6, opt, &m, sizeof(m)) == 0 ? 0 : errno;
}

void bind_any(int fd, int family) {
  struct sockaddr_storage ss;
  memset(&ss, 0, sizeof(ss));
  ss.ss_family = family;
  socklen_t len = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
  assert(bind(fd, (struct sockaddr*)&ss, len) == 0);
}

void test_v4(void) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  bind_any(fd, AF_INET);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.101") == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == EADDRINUSE);
  assert(set4(fd, IP_DROP_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4(fd, IP_DROP_MEMBERSHIP, "227.1.1.100") == EADDRNOTAVAIL);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == 0);

  struct ip_mreq m;
  socklen_t len = sizeof(m);
  assert(getsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, &len) == -1);
  assert(errno == ENOPROTOOPT);

  struct ip_mreq small = mreq4("227.1.1.102");
  assert(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &small, 4) == -1);
  assert(errno == EINVAL);
  close(fd);

  // Joined before bind(): the membership is in effect once bound.
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == EADDRINUSE);
  bind_any(fd, AF_INET);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == EADDRINUSE);
  assert(set4(fd, IP_DROP_MEMBERSHIP, "227.1.1.100") == 0);
  close(fd);
}

void test_v6(void) {
  int fd = socket(AF_INET6, SOCK_DGRAM, 0);
  bind_any(fd, AF_INET6);
  int err = set6(fd, IPV6_JOIN_GROUP, "ff02::1:3");
  if (err == ENODEV) {
    // No IPv6 multicast route on this host (e.g. a container without IPv6).
    close(fd);
    return;
  }
  assert(err == 0);
  assert(set6(fd, IPV6_JOIN_GROUP, "ff02::1:3") == EADDRINUSE);
  assert(set6(fd, IPV6_LEAVE_GROUP, "ff02::1:3") == 0);
  assert(set6(fd, IPV6_LEAVE_GROUP, "ff02::1:3") == EADDRNOTAVAIL);
  close(fd);
}

int main(void) {
  test_v4();
  test_v6();
  printf("done\n");
  return 0;
}
