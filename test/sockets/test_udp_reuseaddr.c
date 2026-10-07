/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * SO_REUSEADDR / SO_REUSEPORT on datagram sockets actually take effect at
 * bind: two sockets with the option set may share a port, and a third without
 * it is refused with EADDRINUSE. Plain POSIX, also runs natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int get_int(int fd, int opt) {
  int val = -1;
  socklen_t len = sizeof(val);
  assert(getsockopt(fd, SOL_SOCKET, opt, &val, &len) == 0);
  return val;
}

void set_int(int fd, int opt, int val) {
  assert(setsockopt(fd, SOL_SOCKET, opt, &val, sizeof(val)) == 0);
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

void check(int opt) {
  int a = socket(AF_INET, SOCK_DGRAM, 0);
  int b = socket(AF_INET, SOCK_DGRAM, 0);
  int c = socket(AF_INET, SOCK_DGRAM, 0);
  assert(a >= 0 && b >= 0 && c >= 0);

  assert(get_int(a, opt) == 0);
  set_int(a, opt, 1);
  set_int(b, opt, 1);
  assert(get_int(a, opt) == 1);

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
  check(SO_REUSEADDR);
  check(SO_REUSEPORT);
  printf("done\n");
  return 0;
}
