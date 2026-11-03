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
 * with EADDRINUSE. Plain POSIX, also runs natively.
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
  printf("done\n");
  return 0;
}
