/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Self-contained IPv6 UDP loopback echo over ::1. Mirrors the IPv4 UDP test but
 * with AF_INET6/sockaddr_in6: the server binds(:0)+getsockname, the client
 * sends a datagram, the server echoes it back to the sender. Plain POSIX; also
 * builds and runs natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int server_fd = -1;
int client_fd = -1;
struct sockaddr_in6 dest;
bool ping_sent = false;
bool pong_sent = false;

void set_nonblocking(int fd) { fcntl(fd, F_SETFL, O_NONBLOCK); }

void test_success(void) {
  printf("UDP IPV6 PASS\n");
  if (server_fd >= 0) close(server_fd);
  if (client_fd >= 0) close(client_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(server_fd, &fdr);
  FD_SET(client_fd, &fdr);
  FD_SET(client_fd, &fdw);
  select(64, &fdr, &fdw, NULL, &tv);

  if (!ping_sent && FD_ISSET(client_fd, &fdw)) {
    if (sendto(client_fd, "ping", 4, 0, (struct sockaddr*)&dest, sizeof(dest)) == 4) {
      ping_sent = true;
    }
  }

  if (!pong_sent && FD_ISSET(server_fd, &fdr)) {
    char buf[4];
    struct sockaddr_in6 src;
    socklen_t sl = sizeof(src);
    ssize_t n = recvfrom(server_fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &sl);
    if (n == 4 && memcmp(buf, "ping", 4) == 0) {
      char ip[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &src.sin6_addr, ip, sizeof(ip));
      printf("server got ping from [%s]:%u\n", ip, (unsigned)ntohs(src.sin6_port));
      sendto(server_fd, "pong", 4, 0, (struct sockaddr*)&src, sl);
      pong_sent = true;
    }
  }

  if (ping_sent && FD_ISSET(client_fd, &fdr)) {
    char buf[4];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n == 4 && memcmp(buf, "pong", 4) == 0) {
      test_success();
    }
  }
}

int main(void) {
  server_fd = socket(AF_INET6, SOCK_DGRAM, 0);
  client_fd = socket(AF_INET6, SOCK_DGRAM, 0);
  assert(server_fd >= 0 && client_fd >= 0);

  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(0); // ephemeral
  assert(inet_pton(AF_INET6, "::1", &addr.sin6_addr) == 1);
  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("bind");
    return 1;
  }

  struct sockaddr_in6 la;
  socklen_t ll = sizeof(la);
  if (getsockname(server_fd, (struct sockaddr*)&la, &ll) != 0) {
    perror("getsockname");
    return 1;
  }
  assert(la.sin6_family == AF_INET6);
  assert(ntohs(la.sin6_port) != 0);
  printf("listening on [::1]:%u\n", (unsigned)ntohs(la.sin6_port));

  set_nonblocking(server_fd);
  set_nonblocking(client_fd);

  memset(&dest, 0, sizeof(dest));
  dest.sin6_family = AF_INET6;
  dest.sin6_port = la.sin6_port;
  assert(inet_pton(AF_INET6, "::1", &dest.sin6_addr) == 1);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (1) {
    main_loop();
    usleep(1000);
  }
#endif
  return 0;
}
