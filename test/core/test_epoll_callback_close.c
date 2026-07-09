/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A registered callback keeps the runtime alive only while its epoll can still
 * fire. Closing the watched fd makes the set terminal (nothing it watches can
 * become ready again), so the keepalive is dropped and the process exits with no
 * explicit unregister - here over a pipe, exercising the PIPEFS close -> wake ->
 * evict path (the same property the sockets test relies on for SOCKFS).
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, rfd, wfd;

static void on_ready(int epfd, struct epoll_event* ev, int n, void* ud) {
  assert(n == 1 && (ev[0].events & EPOLLIN));
  char b[1];
  assert(read(rfd, b, 1) == 1);
  printf("done\n");
  // No unregister: closing the watched fd alone must let the runtime exit.
  close(rfd);
  close(wfd);
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

  assert(emscripten_epoll_set_callback(ep, 4, on_ready, 0) == 0);
  assert(write(wfd, "x", 1) == 1);
  return 0;
}
