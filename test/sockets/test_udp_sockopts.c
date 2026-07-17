/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * UDP multicast socket options. Exercises IP_MULTICAST_TTL / IP_MULTICAST_LOOP
 * on an AF_INET datagram socket and IPV6_MULTICAST_HOPS / IPV6_MULTICAST_LOOP
 * on an AF_INET6 one: reading an option that was never set returns the POSIX
 * default (TTL/HOPS 1, LOOP 1), and a set is observable by a subsequent get.
 * Plain POSIX, also runs natively.
 */

#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

static int get_int(int fd, int level, int opt) {
  int val = -1;
  socklen_t len = sizeof(val);
  assert(getsockopt(fd, level, opt, &val, &len) == 0);
  return val;
}

static void set_int(int fd, int level, int opt, int val) {
  assert(setsockopt(fd, level, opt, &val, sizeof(val)) == 0);
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
  printf("UDP SOCKOPTS PASS\n");
  return 0;
}
