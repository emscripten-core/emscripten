/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * The richer epoll semantics, all exercised non-blocking (timeout 0) over pipes:
 * EPOLLONESHOT (fire once, re-arm with MOD), EPOLLET (edge-triggered), the
 * EPOLLEXCLUSIVE ctl restriction and its round-robin single-wakeup across epolls
 * watching one fd, nesting one epoll inside another, ELOOP rejection of cycles
 * and over-deep chains, and auto-removal of a registration whose fd is closed.
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

static void test_exclusive_wakeup(void) {
  // One fd watched by two epolls with EPOLLEXCLUSIVE: each readiness edge wakes
  // only one of them, rotating - not both (no thundering herd). Edge-triggered so
  // a delivered item is not re-listed, making "who was woken" unambiguous.
  int epA = epoll_create1(0), epB = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  struct epoll_event ev = { .events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE };
  ev.data.fd = p[0];
  assert(epoll_ctl(epA, EPOLL_CTL_ADD, p[0], &ev) == 0);
  assert(epoll_ctl(epB, EPOLL_CTL_ADD, p[0], &ev) == 0);

  assert(write(p[1], "x", 1) == 1); // first edge -> exactly one epoll (epA)
  assert(ready(epA) == 1);
  assert(ready(epB) == 0);

  assert(write(p[1], "y", 1) == 1); // next edge -> the other (epB), round-robin
  assert(ready(epA) == 0);
  assert(ready(epB) == 1);

  close(epA); close(epB); close(p[0]); close(p[1]);
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

static void test_eloop(void) {
  struct epoll_event ev = { .events = EPOLLIN };

  // A direct cycle is rejected: a watches b, then b watching a closes the loop.
  int a = epoll_create1(0), b = epoll_create1(0);
  ev.data.fd = b;
  assert(epoll_ctl(a, EPOLL_CTL_ADD, b, &ev) == 0);
  ev.data.fd = a;
  assert(epoll_ctl(b, EPOLL_CTL_ADD, a, &ev) == -1 && errno == ELOOP);
  close(a); close(b);

  // A chain six epolls deep is one level too far. Build e[4]->e[5] ... e[1]->e[2]
  // (all accepted), then adding e[1] into e[0] would make a 6-level chain.
  int e[6];
  for (int i = 0; i < 6; i++) e[i] = epoll_create1(0);
  for (int i = 5; i >= 2; i--) {
    ev.data.fd = e[i];
    assert(epoll_ctl(e[i - 1], EPOLL_CTL_ADD, e[i], &ev) == 0);
  }
  ev.data.fd = e[1];
  assert(epoll_ctl(e[0], EPOLL_CTL_ADD, e[1], &ev) == -1 && errno == ELOOP);
  for (int i = 0; i < 6; i++) close(e[i]);
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
  test_exclusive_wakeup();
  test_nesting();
  test_eloop();
  test_autoremove();
  printf("EPOLL ADVANCED PASS\n");
  return 0;
}
