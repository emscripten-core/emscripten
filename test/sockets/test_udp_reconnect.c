/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * POSIX connect() on a connected UDP socket replaces the peer and keeps the
 * local address, across repeated re-connects. Checked for an explicit
 * bind(127.0.0.1:0) and an auto-bound socket, in both directions of traffic.
 * A multicast group joined before the re-connects is still joined after.
 */

#include <arpa/inet.h>
#include <assert.h>
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

#define CASES 2

int client_fd[CASES];
int peer_a_fd[CASES];
int peer_b_fd[CASES];
bool client_got[CASES];
bool peer_b_got[CASES];

int udp_socket(void) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);
  return fd;
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

int bound_socket(void) {
  int fd = udp_socket();
  struct sockaddr_in addr = loopback(0);
  assert(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);
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

void setup_case(int i, bool explicit_bind) {
  peer_a_fd[i] = bound_socket();
  peer_b_fd[i] = bound_socket();
  struct sockaddr_in a = local_addr(peer_a_fd[i]);
  struct sockaddr_in b = local_addr(peer_b_fd[i]);

  if (explicit_bind) {
    client_fd[i] = bound_socket();
  } else {
    client_fd[i] = udp_socket();
    fcntl(client_fd[i], F_SETFL, O_NONBLOCK);
  }

  do_connect(client_fd[i], a);
  // A send forces the auto-bind on runtimes that defer it past connect().
  assert(send(client_fd[i], "to-a", 4, 0) == 4);
  struct sockaddr_in before = local_addr(client_fd[i]);
  assert(membership(client_fd[i], IP_ADD_MEMBERSHIP) == 0);

  // Re-connect several times (A -> B -> A -> B): the first re-connect and the
  // ones after it must all keep the port.
  struct sockaddr_in peers[] = {b, a, b};
  for (int j = 0; j < 3; j++) {
    do_connect(client_fd[i], peers[j]);
    struct sockaddr_in after = local_addr(client_fd[i]);
    printf("%s: port before reconnect %u, after reconnect %d %u\n",
           explicit_bind ? "bind(:0)" : "auto-bind",
           (unsigned)ntohs(before.sin_port), j + 1, (unsigned)ntohs(after.sin_port));
    assert(before.sin_port == after.sin_port);
  }
  assert(membership(client_fd[i], IP_DROP_MEMBERSHIP) == 0);

  // Peer B only accepts datagrams from the client's original port.
  do_connect(peer_b_fd[i], loopback(ntohs(before.sin_port)));
  assert(send(peer_b_fd[i], "to-client", 9, 0) == 9);
  assert(send(client_fd[i], "to-b", 4, 0) == 4);
}

void finish(void) {
  for (int i = 0; i < CASES; i++) {
    close(client_fd[i]);
    close(peer_a_fd[i]);
    close(peer_b_fd[i]);
  }
  printf("done\n");
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void expect(int fd, const char* want, bool* got) {
  char buf[16];
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  if (n < 0) return;
  assert(n == (ssize_t)strlen(want) && memcmp(buf, want, n) == 0);
  *got = true;
}

void main_loop(void) {
  fd_set fdr;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  int nfds = 0;
  for (int i = 0; i < CASES; i++) {
    FD_SET(client_fd[i], &fdr);
    FD_SET(peer_b_fd[i], &fdr);
    if (client_fd[i] >= nfds) nfds = client_fd[i] + 1;
    if (peer_b_fd[i] >= nfds) nfds = peer_b_fd[i] + 1;
  }
  select(nfds, &fdr, NULL, NULL, &tv);

  bool all = true;
  for (int i = 0; i < CASES; i++) {
    if (!client_got[i] && FD_ISSET(client_fd[i], &fdr)) expect(client_fd[i], "to-client", &client_got[i]);
    if (!peer_b_got[i] && FD_ISSET(peer_b_fd[i], &fdr)) expect(peer_b_fd[i], "to-b", &peer_b_got[i]);
    all = all && client_got[i] && peer_b_got[i];
  }
  if (all) finish();
}

int main(void) {
  setup_case(0, true);
  setup_case(1, false);
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
