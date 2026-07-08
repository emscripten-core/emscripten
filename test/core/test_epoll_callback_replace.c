/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_epoll_set_callback registration semantics: there is at most one
 * callback per epoll, so a second register replaces the first (callbacks do not
 * stack), and a NULL callback unregisters regardless of maxevents (including 0).
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

static int ep, rfd, wfd;
static int c1, c2;

static void cb1(int e, struct epoll_event* ev, int n, void* ud) { c1++; }

static void cb2(int e, struct epoll_event* ev, int n, void* ud) {
  c2++;
  assert(c1 == 0); // the replaced callback must never have fired
  char b[1];
  assert(read(rfd, b, 1) == 1); // drain

  // Unregister with maxevents 0: it is ignored when clearing. Make the set ready
  // again to prove no further delivery happens.
  assert(emscripten_epoll_set_callback(ep, 0, 0, 0) == 0);
  assert(write(wfd, "x", 1) == 1);
  printf("done\n");
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);

  // A non-epoll fd is rejected with a positive EBADF.
  assert(emscripten_epoll_set_callback(rfd, 4, cb1, 0) == EBADF);

  // Register then immediately replace, before any tick runs: only cb2 is armed.
  assert(emscripten_epoll_set_callback(ep, 4, cb1, 0) == 0);
  assert(emscripten_epoll_set_callback(ep, 4, cb2, 0) == 0);
  assert(write(wfd, "x", 1) == 1); // delivered on the next tick, to cb2 only
  return 0;
}
