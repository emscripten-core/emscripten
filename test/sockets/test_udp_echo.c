/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Self-contained UDP loopback echo. A server and a client live in one process,
 * both non-blocking, driven by select in the main loop. The server binds(:0)
 * and reads its assigned port via getsockname (synchronous), the client sends
 * a datagram to it, the server echoes it back to the sender. This is plain
 * POSIX and also builds and runs natively.
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
struct sockaddr_in dest;
bool ping_sent = false;
bool pong_sent = false;

void set_nonblocking(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);
}

void test_success(void) {
  printf("UDP ECHO PASS\n");
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

  // client: send ping
  if (!ping_sent && FD_ISSET(client_fd, &fdw)) {
    if (sendto(client_fd, "ping", 4, 0, (struct sockaddr*)&dest, sizeof(dest)) == 4) {
      ping_sent = true;
    }
  }

  // server: echo ping -> pong back to the sender
  if (!pong_sent && FD_ISSET(server_fd, &fdr)) {
    char buf[4];
    struct sockaddr_in src;
    socklen_t sl = sizeof(src);
    ssize_t n = recvfrom(server_fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &sl);
    if (n == 4 && memcmp(buf, "ping", 4) == 0) {
      printf("server got ping from %s:%u\n", inet_ntoa(src.sin_addr), (unsigned)ntohs(src.sin_port));
      sendto(server_fd, "pong", 4, 0, (struct sockaddr*)&src, sl);
      pong_sent = true;
    }
  }

  // client: receive pong
  if (ping_sent && FD_ISSET(client_fd, &fdr)) {
    char buf[4];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n == 4 && memcmp(buf, "pong", 4) == 0) {
      test_success();
    }
  }
}

int main(void) {
  server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  client_fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(server_fd >= 0 && client_fd >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0); // ephemeral
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("bind");
    return 1;
  }

  // The OS-assigned ephemeral port must be readable synchronously.
  struct sockaddr_in la;
  socklen_t ll = sizeof(la);
  if (getsockname(server_fd, (struct sockaddr*)&la, &ll) != 0) {
    perror("getsockname");
    return 1;
  }
  assert(ntohs(la.sin_port) != 0);
  printf("listening on 127.0.0.1:%u\n", (unsigned)ntohs(la.sin_port));

  // Datagram socket options round-trip on the bound socket.
  int ttl = 64, on = 1, rcv = 131072, got;
  socklen_t gl = sizeof(got);
  assert(setsockopt(server_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == 0);
  assert(setsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == 0);
  assert(setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &rcv, sizeof(rcv)) == 0);
  assert(getsockopt(server_fd, IPPROTO_IP, IP_TTL, &got, &gl) == 0 && got == 64);
  assert(getsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &got, &gl) == 0 && got != 0);
  assert(getsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &got, &gl) == 0 && got > 0);

  set_nonblocking(server_fd);
  set_nonblocking(client_fd);

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = la.sin_port;
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

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
