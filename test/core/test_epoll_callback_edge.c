/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * EPOLLET on the callback path: an edge-triggered fd delivers once per edge. It
 * must NOT re-fire while it stays continuously readable (the byte is never
 * drained), and it fires again only on a fresh edge (a new write).
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, rfd, wfd, fires;

static void second_edge(void* arg) {
  // The fd stayed readable the whole time (fire 1 did not drain it), yet the
  // edge-triggered callback did not re-fire. A LEVEL fd would have re-delivered
  // (and spun) by now, so fires==1 here is the EPOLLET once-per-edge guarantee.
  assert(fires == 1);
  assert(write(wfd, "y", 1) == 1); // a fresh edge -> exactly one more delivery
}

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1);
  assert(ev[0].data.fd == rfd);
  assert(ev[0].events & EPOLLIN);
  fires++;

  if (fires == 1) {
    // Do NOT drain: leave the fd readable, then check it stays silent and poke a
    // fresh edge.
    emscripten_async_call(second_edge, NULL, 0);
    return;
  }

  assert(fires == 2);
  char b[2];
  assert(read(rfd, b, 2) == 2); // drain both bytes
  assert(emscripten_epoll_set_callback(ep, NULL, NULL) == 0);
  printf("done\n");
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN | EPOLLET };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);

  assert(emscripten_epoll_set_callback(ep, on_ready, 0) == 0);
  assert(write(wfd, "x", 1) == 1); // first edge
  return 0;
}
