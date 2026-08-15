/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Listener registration identity: a listener is keyed by (callback, thread), so
 * re-adding the same callback replaces it (just updating userdata, no
 * stacking), and emscripten_epoll_remove_listener removes by callback identity
 * (ENOENT when absent, EBADF on a non-epoll fd).
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

static int ep, rfd, wfd;
static int fires;

static void on_ready(void* ud) {
  // Re-added with updated userdata: only the second registration's userdata is
  // ever delivered, exactly once per collected batch.
  assert((long)ud == 2);
  fires++;
  assert(fires == 1);
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1);
  char b[1];
  assert(read(rfd, b, 1) == 1); // drain

  // Remove, then make the set ready again to prove no further delivery happens.
  assert(emscripten_epoll_remove_listener(ep, on_ready) == 0);
  assert(emscripten_epoll_remove_listener(ep, on_ready) == ENOENT);
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
  assert(emscripten_epoll_add_listener(rfd, on_ready, 0) == EBADF);
  assert(emscripten_epoll_remove_listener(rfd, on_ready) == EBADF);
  // Removing a never-added listener is ENOENT.
  assert(emscripten_epoll_remove_listener(ep, on_ready) == ENOENT);

  // Add then immediately re-add the same identity, before any tick runs: one
  // registration, carrying the updated userdata.
  assert(emscripten_epoll_add_listener(ep, on_ready, (void*)1) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, (void*)2) == 0);
  assert(write(wfd, "x", 1) == 1); // delivered on the next tick, once
  return 0;
}
