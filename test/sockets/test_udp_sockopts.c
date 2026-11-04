/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * UDP socket options. Exercises IP_MULTICAST_TTL / IP_MULTICAST_LOOP on an
 * AF_INET datagram socket and IPV6_MULTICAST_HOPS / IPV6_MULTICAST_LOOP on an
 * AF_INET6 one: reading an option that was never set returns the POSIX
 * default (TTL/HOPS 1, LOOP 1), and a set is observable by a subsequent get.
 * Then SO_REUSEADDR / SO_REUSEPORT, which take effect at bind: two sockets
 * with the option set may share a port, and a third without it is refused
 * with EADDRINUSE. Finally the set-only IP_ADD_MEMBERSHIP / IP_DROP_MEMBERSHIP
 * and IPv6 equivalents reach the kernel: a duplicate join fails with
 * EADDRINUSE, leaving an unjoined group with EADDRNOTAVAIL, a join made before
 * bind() is applied once bound, a leave that names no interface finds the
 * group, and a join the kernel rejects leaves nothing behind. Plain POSIX, also
 * runs natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int get_int(int fd, int level, int opt) {
  int val = -1;
  socklen_t len = sizeof(val);
  assert(getsockopt(fd, level, opt, &val, &len) == 0);
  return val;
}

void set_int(int fd, int level, int opt, int val) {
  assert(setsockopt(fd, level, opt, &val, sizeof(val)) == 0);
}

int bind_any(int fd, in_port_t port) {
  struct sockaddr_in sa = {.sin_family = AF_INET, .sin_port = port, .sin_addr.s_addr = htonl(INADDR_ANY)};
  return bind(fd, (struct sockaddr*)&sa, sizeof(sa));
}

in_port_t bound_port(int fd) {
  struct sockaddr_in sa;
  socklen_t len = sizeof(sa);
  assert(getsockname(fd, (struct sockaddr*)&sa, &len) == 0);
  return sa.sin_port;
}

void check_reuse(int opt) {
  int a = socket(AF_INET, SOCK_DGRAM, 0);
  int b = socket(AF_INET, SOCK_DGRAM, 0);
  int c = socket(AF_INET, SOCK_DGRAM, 0);
  assert(a >= 0 && b >= 0 && c >= 0);

  assert(get_int(a, SOL_SOCKET, opt) == 0);
  set_int(a, SOL_SOCKET, opt, 1);
  set_int(b, SOL_SOCKET, opt, 1);
  assert(get_int(a, SOL_SOCKET, opt) == 1);

  assert(bind_any(a, 0) == 0);
  in_port_t port = bound_port(a);
  assert(port != 0);
  // Sharing is allowed between sockets that both set the option...
  assert(bind_any(b, port) == 0);
  assert(bound_port(b) == port);
  // ...and refused for one that did not.
  assert(bind_any(c, port) == -1 && errno == EADDRINUSE);

  close(a);
  close(b);
  close(c);
}

int set4_on(int fd, int opt, const char* group, const char* iface) {
  struct ip_mreq m = {0};
  inet_pton(AF_INET, group, &m.imr_multiaddr);
  if (iface) {
    inet_pton(AF_INET, iface, &m.imr_interface);
  }
  return setsockopt(fd, IPPROTO_IP, opt, &m, sizeof(m)) == 0 ? 0 : errno;
}

int set4(int fd, int opt, const char* group) {
  return set4_on(fd, opt, group, "127.0.0.1");
}

int set6(int fd, int opt, const char* group) {
  struct ipv6_mreq m = {0};
  inet_pton(AF_INET6, group, &m.ipv6mr_multiaddr);
  return setsockopt(fd, IPPROTO_IPV6, opt, &m, sizeof(m)) == 0 ? 0 : errno;
}

void check_membership(void) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(bind_any(fd, 0) == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.101") == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == EADDRINUSE);
  assert(set4(fd, IP_DROP_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4(fd, IP_DROP_MEMBERSHIP, "227.1.1.100") == EADDRNOTAVAIL);

  struct ip_mreq m;
  socklen_t len = sizeof(m);
  assert(getsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, &len) == -1);
  assert(errno == ENOPROTOOPT);
  assert(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, 4) == -1);
  assert(errno == EINVAL);
  close(fd);

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == EADDRINUSE);
  assert(bind_any(fd, 0) == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == EADDRINUSE);
  assert(set4(fd, IP_DROP_MEMBERSHIP, "227.1.1.100") == 0);
  close(fd);

  // A leave that names no interface matches a join on a specific one.
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(bind_any(fd, 0) == 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "227.1.1.100") == 0);
  assert(set4_on(fd, IP_DROP_MEMBERSHIP, "227.1.1.100", NULL) == 0);
  assert(set4_on(fd, IP_DROP_MEMBERSHIP, "227.1.1.100", NULL) == EADDRNOTAVAIL);
  close(fd);

  // A non-multicast group is refused even before bind().
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(set4(fd, IP_ADD_MEMBERSHIP, "10.0.0.1") == EINVAL);
  close(fd);
  fd = socket(AF_INET6, SOCK_DGRAM, 0);
  assert(set6(fd, IPV6_JOIN_GROUP, "::1") == EINVAL);
  close(fd);

  // A join on an interface address no host has (192.0.2.0/24 is reserved for
  // documentation) can only fail once bound, but must not count as joined. The
  // error itself differs between kernels.
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  set4_on(fd, IP_ADD_MEMBERSHIP, "227.1.1.100", "192.0.2.1");
  assert(bind_any(fd, 0) == 0);
  int rejected = set4_on(fd, IP_ADD_MEMBERSHIP, "227.1.1.100", "192.0.2.1");
  assert(rejected != 0 && rejected != EADDRINUSE);
  close(fd);

  fd = socket(AF_INET6, SOCK_DGRAM, 0);
  struct sockaddr_in6 sa6 = {.sin6_family = AF_INET6};
  assert(bind(fd, (struct sockaddr*)&sa6, sizeof(sa6)) == 0);
  int err = set6(fd, IPV6_JOIN_GROUP, "ff02::1:3");
  // ENODEV: no IPv6 multicast route on this host (e.g. a container).
  if (err != ENODEV) {
    assert(err == 0);
    assert(set6(fd, IPV6_JOIN_GROUP, "ff02::1:3") == EADDRINUSE);
    assert(set6(fd, IPV6_LEAVE_GROUP, "ff02::1:3") == 0);
    assert(set6(fd, IPV6_LEAVE_GROUP, "ff02::1:3") == EADDRNOTAVAIL);
  }
  close(fd);
}

int main(void) {
  int v4 = socket(AF_INET, SOCK_DGRAM, 0);
  assert(v4 >= 0);

  // Defaults are readable without any prior set.
  assert(get_int(v4, IPPROTO_IP, IP_MULTICAST_TTL) == 1);
  assert(get_int(v4, IPPROTO_IP, IP_MULTICAST_LOOP) == 1);

  set_int(v4, IPPROTO_IP, IP_MULTICAST_TTL, 5);
  assert(get_int(v4, IPPROTO_IP, IP_MULTICAST_TTL) == 5);

  set_int(v4, IPPROTO_IP, IP_MULTICAST_LOOP, 0);
  assert(get_int(v4, IPPROTO_IP, IP_MULTICAST_LOOP) == 0);
  set_int(v4, IPPROTO_IP, IP_MULTICAST_LOOP, 1);
  assert(get_int(v4, IPPROTO_IP, IP_MULTICAST_LOOP) == 1);

  int v6 = socket(AF_INET6, SOCK_DGRAM, 0);
  assert(v6 >= 0);

  assert(get_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_HOPS) == 1);
  assert(get_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP) == 1);

  set_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, 3);
  assert(get_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_HOPS) == 3);

  set_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, 0);
  assert(get_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP) == 0);
  set_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, 1);
  assert(get_int(v6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP) == 1);

  // An out-of-range value is rejected with EINVAL rather than silently
  // ignored. Bind first: the range is validated when the option is applied to
  // the live socket.
  int b4 = socket(AF_INET, SOCK_DGRAM, 0);
  assert(b4 >= 0);
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  assert(bind(b4, (struct sockaddr*)&addr, sizeof(addr)) == 0);

  int bad = 300;
  errno = 0;
  assert(setsockopt(b4, IPPROTO_IP, IP_MULTICAST_TTL, &bad, sizeof(bad)) == -1);
  assert(errno == EINVAL);
  // The rejected value is dropped, so a later valid set still succeeds.
  set_int(b4, IPPROTO_IP, IP_MULTICAST_LOOP, 0);
  assert(get_int(b4, IPPROTO_IP, IP_MULTICAST_LOOP) == 0);
  close(b4);

  close(v4);
  close(v6);

  check_reuse(SO_REUSEADDR);
  check_reuse(SO_REUSEPORT);
  check_membership();
  printf("done\n");
  return 0;
}
