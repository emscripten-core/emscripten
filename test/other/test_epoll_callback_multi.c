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
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, rfd[2];
static int seen[2];
static int fires_a, fires_b, collected;

static int idx(int fd) {
  for (int i = 0; i < 2; i++) if (rfd[i] == fd) return i;
  return -1;
}

static void collect(void) {
  struct epoll_event ev[1];
  int n = epoll_wait(ep, ev, 1, 0); // collect at most one per fire
  if (n == 1) {
    int i = idx(ev[0].data.fd);
    assert(i >= 0 && !seen[i]); // disjoint: each fd collected exactly once
    seen[i] = 1;
    char b[1];
    assert(read(rfd[i], b, 1) == 1); // drain so it is no longer ready
    collected++;
  }
}

static void listener_a(void* ud) { fires_a++; collect(); }
static void listener_b(void* ud) { fires_b++; collect(); }

static void check(void* ud) {
  // Both listeners were woken by the same readiness (broadcast) and the split
  // was one event each (load balancing).
  assert(collected == 2 && seen[0] && seen[1]);
  assert(fires_a == 1 && fires_b == 1);
  assert(emscripten_epoll_remove_listener(ep, listener_a) == 0);
  assert(emscripten_epoll_remove_listener(ep, listener_b) == 0);
  printf("done\n");
}

int main(void) {
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

  assert(emscripten_epoll_add_listener(ep, listener_a, 0) == 0);
  assert(emscripten_epoll_add_listener(ep, listener_b, 0) == 0);
  // Both fds are already ready: A's tick collects one, B's collects the other,
  // then a macrotask verifies the exact one-each split before removing both.
  emscripten_async_call(check, NULL, 0);
  return 0;
}
