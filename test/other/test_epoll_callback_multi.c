/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Multiple listeners on one epoll: every listener is signalled while
 * uncollected ready events remain (broadcast), and collectors race over the
 * shared ready list, so each event is collected exactly once (load balancing).
 * Two listeners each collecting one event per fire split two ready fds one
 * each: A's first tick takes one, B's tick takes the other, and A's re-fire
 * finds nothing left so it stays silent.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

int ep, ep_b, rfd[2];
int seen[2];
int fires_a, fires_b, collected;

int idx(int fd) {
  for (int i = 0; i < 2; i++) if (rfd[i] == fd) return i;
  return -1;
}

void collect(int epfd) {
  struct epoll_event ev[1];
  int n = epoll_wait(epfd, ev, 1, 0); // collect at most one per fire
  if (n == 1) {
    int i = idx(ev[0].data.fd);
    assert(i >= 0 && !seen[i]); // disjoint: each fd collected exactly once
    seen[i] = 1;
    char b[1];
    assert(read(rfd[i], b, 1) == 1); // drain so it is no longer ready
    collected++;
  }
}

void listener_a(int epfd, void* ud) {
  assert(epfd == ep);
  fires_a++;
  collect(epfd);
}

void listener_b(int epfd, void* ud) {
  assert(epfd == ep_b);
  fires_b++;
  collect(epfd);
}

void check(void* ud) {
  // Both listeners were woken by the same readiness (broadcast) and the split
  // was one event each (load balancing).
  assert(collected == 2 && seen[0] && seen[1]);
  assert(fires_a == 1 && fires_b == 1);
  assert(emscripten_epoll_listener_remove(ep, listener_a, 0) == 0);
  assert(emscripten_epoll_listener_remove(ep_b, listener_b, 0) == 0);
  printf("done\n");
}

int main() {
  ep = epoll_create1(0);
  for (int i = 0; i < 2; i++) {
    int p[2];
    assert(pipe(p) == 0);
    rfd[i] = p[0];
    assert(write(p[1], "x", 1) == 1); // read end readable (level)
    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.fd = rfd[i];
    assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd[i], &ev) == 0);
  }

  assert(emscripten_epoll_listener_add(ep, listener_a, 0) == 0);
#ifdef MODE_DUP
  ep_b = dup(ep);
  assert(ep_b >= 0 && ep_b != ep);
#else
  ep_b = ep;
#endif
  assert(emscripten_epoll_listener_add(ep_b, listener_b, 0) == 0);
  // Both fds are already ready: A's delivery collects one, B's the other. The
  // deliveries are immediates queued by listener_add, so an immediate queued
  // after them runs once both have, and verifies the exact one-each split.
  emscripten_set_immediate(check, NULL);
  return 0;
}
