/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Connected UDP semantics. A client connect()s to a loopback server, which
 * means: sendto() with an explicit address must fail with EISCONN, send()
 * without an address goes to the peer, and datagrams from anyone other than
 * the peer are not delivered. A third "other" socket sends junk to the client
 * to prove that filtering. Plain POSIX, also runs natively.
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
int other_fd = -1;
struct sockaddr_in server_addr;
bool echoed = false;

void set_nonblocking(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);
}

void test_success(void) {
  printf("UDP CONNECT PASS\n");
  if (server_fd >= 0) close(server_fd);
  if (client_fd >= 0) close(client_fd);
  if (other_fd >= 0) close(other_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void main_loop(void) {
  fd_set fdr;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_SET(server_fd, &fdr);
  FD_SET(client_fd, &fdr);
  select(64, &fdr, NULL, NULL, &tv);

  // server: receive the peer's ping and echo a pong back to it
  if (!echoed && FD_ISSET(server_fd, &fdr)) {
    char buf[8];
    struct sockaddr_in src;
    socklen_t sl = sizeof(src);
    ssize_t n = recvfrom(server_fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &sl);
    if (n == 4 && memcmp(buf, "ping", 4) == 0) {
      sendto(server_fd, "pong", 4, 0, (struct sockaddr*)&src, sl);
      echoed = true;
    }
  }

  // client: the only datagram it should ever see is the peer's pong, never the
  // "junk" sent by the unrelated socket.
  if (FD_ISSET(client_fd, &fdr)) {
    char buf[8];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n == 4 && memcmp(buf, "pong", 4) == 0) {
      test_success();
    } else if (n > 0) {
      printf("client received non-peer datagram (%.*s)\n", (int)n, buf);
      assert(false && "client received non-peer datagram");
    }
  }
}

int main(void) {
  server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(server_fd >= 0);
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(0);
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
  assert(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0);
  socklen_t sl = sizeof(server_addr);
  assert(getsockname(server_fd, (struct sockaddr*)&server_addr, &sl) == 0);
  set_nonblocking(server_fd);

  client_fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0);

  // sendto() with an explicit destination on a connected datagram socket fails.
  ssize_t r = sendto(client_fd, "x", 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
  assert(r == -1 && errno == EISCONN);

  set_nonblocking(client_fd);
  assert(send(client_fd, "ping", 4, 0) == 4);

  // Learn the client's auto-bound port so the "other" socket can target it.
  struct sockaddr_in client_addr;
  socklen_t cl = sizeof(client_addr);
  assert(getsockname(client_fd, (struct sockaddr*)&client_addr, &cl) == 0);
  assert(ntohs(client_addr.sin_port) != 0);

  other_fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(other_fd >= 0);
  sendto(other_fd, "junk", 4, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));

  printf("connected to 127.0.0.1:%u, client port %u\n",
         (unsigned)ntohs(server_addr.sin_port), (unsigned)ntohs(client_addr.sin_port));

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
