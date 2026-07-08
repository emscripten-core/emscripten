/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Closing a nested (inner) epoll wakes the outer epoll watching it, which
 * re-derives and drops the now-stale registration. An outer callback that
 * watched only the inner then has nothing that can fire, so it stops keeping the
 * runtime alive and the process exits - with no explicit unregister, the same
 * terminal-set property as closing a leaf fd, one level up.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int epA, epB, rfd, wfd;

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(epA, ev, 4, 0) == 1 && ev[0].data.fd == epB);
  printf("done\n");
  close(epB); // inner epoll gone -> outer's only registration becomes terminal
}

int main(void) {
  epA = epoll_create1(0);
  epB = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(epB, EPOLL_CTL_ADD, rfd, &ev) == 0); // leaf in the inner
  ev.data.fd = epB;
  assert(epoll_ctl(epA, EPOLL_CTL_ADD, epB, &ev) == 0); // inner in the outer

  assert(emscripten_epoll_set_callback(epA, on_ready, 0) == 0);
  assert(write(wfd, "x", 1) == 1); // leaf ready -> propagates up to epA's callback
  return 0;
}
