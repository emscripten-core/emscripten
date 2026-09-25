/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A burst of clients connects and settles before the server arms its listener
 * or ever calls accept(): the accept queue has depth one, so every delivery
 * finds exactly one client and accept() then reports EAGAIN, however many are
 * waiting.
 */
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define CLIENTS 3

int ep, srv, clients[CLIENTS];
int fires, accepted;

void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1);
  assert(ev[0].data.fd == srv);
  int n = 0;
  for (;;) {
    int c = accept(srv, NULL, NULL);
    if (c < 0) {
      assert(errno == EAGAIN);
      break;
    }
    close(c);
    n++;
  }
  // Exactly one per delivery: the next client is queued on a later turn.
  assert(n == 1);
  fires++;
  accepted += n;
  if (accepted == CLIENTS) {
    for (int i = 0; i < CLIENTS; i++) close(clients[i]);
    close(srv);
    emscripten_runtime_keepalive_pop();
  }
}

void at_exit(void) {
  assert(fires == CLIENTS);
  assert(accepted == CLIENTS);
  printf("done\n");
}

void arm(void* ud) {
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = srv;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, srv, &ev) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
}

int main(void) {
  atexit(at_exit);
  ep = epoll_create1(0);
  srv = socket(AF_INET, SOCK_STREAM, 0);
  assert(fcntl(srv, F_SETFL, O_NONBLOCK) == 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET; addr.sin_port = htons(0);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(srv, (struct sockaddr*)&addr, sizeof addr) == 0);
  socklen_t l = sizeof addr;
  assert(getsockname(srv, (struct sockaddr*)&addr, &l) == 0);
  assert(listen(srv, 8) == 0);
  // All clients connect, and their connections land, well before anything
  // arms the listener or accepts.
  for (int i = 0; i < CLIENTS; i++) {
    clients[i] = socket(AF_INET, SOCK_STREAM, 0);
    assert(fcntl(clients[i], F_SETFL, O_NONBLOCK) == 0);
    int r = connect(clients[i], (struct sockaddr*)&addr, sizeof addr);
    assert(r == 0 || errno == EINPROGRESS);
  }
  emscripten_set_timeout(arm, 200, 0);
  emscripten_runtime_keepalive_push();
  return 0;
}
