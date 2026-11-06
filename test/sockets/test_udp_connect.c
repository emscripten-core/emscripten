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
 * to prove that filtering. A connect() on a connected socket replaces the peer
 * and keeps the local address across repeated re-connects, for an explicit
 * bind(127.0.0.1:0) and an auto-bound socket, along with any multicast group
 * joined before. Plain POSIX, also runs natively.
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
bool ponged = false;

struct reconnect_case {
  int client, peer_a, peer_b;
  bool client_got, peer_b_got;
} cases[2];

void set_nonblocking(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);
}

struct sockaddr_in loopback(unsigned short port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  return addr;
}

struct sockaddr_in local_addr(int fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  assert(getsockname(fd, (struct sockaddr*)&addr, &len) == 0);
  assert(ntohs(addr.sin_port) != 0);
  return addr;
}

int udp_socket(bool bound) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);
  if (bound) {
    struct sockaddr_in addr = loopback(0);
    assert(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  }
  set_nonblocking(fd);
  return fd;
}

void do_connect(int fd, struct sockaddr_in addr) {
  assert(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
}

int membership(int fd, int opt) {
  struct ip_mreq m = {0};
  inet_pton(AF_INET, "227.1.1.100", &m.imr_multiaddr);
  inet_pton(AF_INET, "127.0.0.1", &m.imr_interface);
  return setsockopt(fd, IPPROTO_IP, opt, &m, sizeof(m));
}

void setup_reconnect(struct reconnect_case* c, bool explicit_bind) {
  c->peer_a = udp_socket(true);
  c->peer_b = udp_socket(true);
  struct sockaddr_in a = local_addr(c->peer_a);
  struct sockaddr_in b = local_addr(c->peer_b);
  c->client = udp_socket(explicit_bind);

  do_connect(c->client, a);
  // A send forces the auto-bind on runtimes that defer it past connect().
  assert(send(c->client, "to-a", 4, 0) == 4);
  struct sockaddr_in before = local_addr(c->client);
  assert(membership(c->client, IP_ADD_MEMBERSHIP) == 0);

  // Re-connect several times (A -> B -> A -> B): the first re-connect and the
  // ones after it must all keep the port.
  struct sockaddr_in peers[] = {b, a, b};
  for (int j = 0; j < 3; j++) {
    do_connect(c->client, peers[j]);
    assert(local_addr(c->client).sin_port == before.sin_port);
  }
  assert(membership(c->client, IP_DROP_MEMBERSHIP) == 0);

  // Peer B only accepts datagrams from the client's original port.
  do_connect(c->peer_b, loopback(ntohs(before.sin_port)));
  assert(send(c->peer_b, "to-client", 9, 0) == 9);
  assert(send(c->client, "to-b", 4, 0) == 4);
}

void expect(int fd, const char* want, bool* got) {
  char buf[16];
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  if (n < 0) return;
  assert(n == (ssize_t)strlen(want) && memcmp(buf, want, n) == 0);
  *got = true;
}

void test_success(void) {
  printf("done\n");
  close(server_fd);
  close(client_fd);
  close(other_fd);
  for (int i = 0; i < 2; i++) {
    close(cases[i].client);
    close(cases[i].peer_a);
    close(cases[i].peer_b);
  }
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
  int nfds = server_fd > client_fd ? server_fd : client_fd;
  for (int i = 0; i < 2; i++) {
    FD_SET(cases[i].client, &fdr);
    FD_SET(cases[i].peer_b, &fdr);
    if (cases[i].client > nfds) nfds = cases[i].client;
    if (cases[i].peer_b > nfds) nfds = cases[i].peer_b;
  }
  select(nfds + 1, &fdr, NULL, NULL, &tv);

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
      ponged = true;
    } else if (n > 0) {
      printf("client received non-peer datagram (%.*s)\n", (int)n, buf);
      assert(false && "client received non-peer datagram");
    }
  }

  bool all = ponged;
  for (int i = 0; i < 2; i++) {
    struct reconnect_case* c = &cases[i];
    if (!c->client_got && FD_ISSET(c->client, &fdr)) expect(c->client, "to-client", &c->client_got);
    if (!c->peer_b_got && FD_ISSET(c->peer_b, &fdr)) expect(c->peer_b, "to-b", &c->peer_b_got);
    all = all && c->client_got && c->peer_b_got;
  }
  if (all) test_success();
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

  setup_reconnect(&cases[0], true);
  setup_reconnect(&cases[1], false);

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
