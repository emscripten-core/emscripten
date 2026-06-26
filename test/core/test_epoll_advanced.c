/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * The richer epoll semantics, all exercised non-blocking (timeout 0) over pipes:
 * EPOLLONESHOT (fire once, re-arm with MOD), EPOLLET (edge-triggered), the
 * EPOLLEXCLUSIVE ctl restriction, nesting one epoll inside another, and
 * auto-removal of a registration whose fd is closed.
 */

#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

static int ready(int ep) {
  struct epoll_event out[4];
  return epoll_wait(ep, out, 4, 0);
}

static void test_oneshot(void) {
  int ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);

  struct epoll_event ev = { .events = EPOLLIN | EPOLLONESHOT };
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);

  assert(write(p[1], "x", 1) == 1);
  assert(ready(ep) == 1);     // fires once
  assert(ready(ep) == 0);     // silent until re-armed, despite still readable

  ev.events = EPOLLIN | EPOLLONESHOT;
  assert(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &ev) == 0);
  assert(ready(ep) == 1);     // re-armed -> fires again

  close(ep); close(p[0]); close(p[1]);
}

static void test_edge(void) {
  int ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);

  struct epoll_event ev = { .events = EPOLLIN | EPOLLET };
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);

  assert(write(p[1], "x", 1) == 1);
  assert(ready(ep) == 1);     // reports on the edge
  assert(ready(ep) == 0);     // not re-reported while continuously ready

  assert(write(p[1], "y", 1) == 1);
  assert(ready(ep) == 1);     // a fresh write is a fresh edge

  close(ep); close(p[0]); close(p[1]);
}

static void test_exclusive(void) {
  int ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);

  struct epoll_event ev = { .events = EPOLLIN | EPOLLEXCLUSIVE };
  ev.data.fd = p[0];
  // EPOLLEXCLUSIVE is accepted at ADD and otherwise functions.
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);
  assert(write(p[1], "x", 1) == 1);
  assert(ready(ep) == 1);
  // EPOLLEXCLUSIVE may not be combined with MOD.
  assert(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &ev) == -1 && errno == EINVAL);

  close(ep); close(p[0]); close(p[1]);
}

static void test_nesting(void) {
  int epA = epoll_create1(0);
  int epB = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = p[0];
  assert(epoll_ctl(epB, EPOLL_CTL_ADD, p[0], &ev) == 0);

  ev.events = EPOLLIN;
  ev.data.fd = epB;
  assert(epoll_ctl(epA, EPOLL_CTL_ADD, epB, &ev) == 0);

  assert(ready(epA) == 0);    // leaf not yet ready -> epB not ready -> epA quiet
  assert(write(p[1], "x", 1) == 1);

  struct epoll_event out[4];
  assert(epoll_wait(epA, out, 4, 0) == 1); // leaf readiness propagates to epA
  assert(out[0].data.fd == epB);
  assert(out[0].events & EPOLLIN);

  close(epA); close(epB); close(p[0]); close(p[1]);
}

static void test_autoremove(void) {
  int ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);

  // Closing the watched fd drops the registration; the wait neither crashes nor
  // reports the dead fd.
  close(p[0]);
  close(p[1]);
  assert(ready(ep) == 0);

  close(ep);
}

int main(void) {
  test_oneshot();
  test_edge();
  test_exclusive();
  test_nesting();
  test_autoremove();
  printf("EPOLL ADVANCED PASS\n");
  return 0;
}
