/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <sys/epoll.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int ep_a, ep_b, rfd, fires;

void on_closed(int epfd, void* ud) {
  assert(0 && "delivery after the registering descriptor closed");
}

void on_ready(int epfd, void* ud) {
  assert(epfd == ep_b);
  assert(ud == &fires);
  struct epoll_event ev;
  assert(epoll_wait(epfd, &ev, 1, 0) == 1 && ev.data.fd == rfd);
  char b;
  assert(read(rfd, &b, 1) == 1);
  fires++;
  assert(emscripten_epoll_listener_remove(epfd, on_ready, ud) == 0);
  assert(close(epfd) == 0);
}

void check(void* ud) {
  assert(fires == 1);
#ifdef MODE_DUP2
  assert(emscripten_epoll_listener_remove(ep_a, on_ready, &fires) == ENOENT);
#endif
  assert(close(ep_a) == 0);
  printf("done\n");
}

int main() {
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  ep_a = epoll_create1(0);
  ep_b = dup(ep_a);
  assert(ep_a >= 0 && ep_b >= 0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep_a, EPOLL_CTL_ADD, rfd, &ev) == 0);
  assert(emscripten_epoll_listener_add(ep_a, on_closed, NULL) == 0);
  assert(emscripten_epoll_listener_add(ep_b, on_ready, &fires) == 0);
  assert(write(p[1], "x", 1) == 1);

  // Cancel queued deliveries before reusing the descriptor number.
#ifdef MODE_DUP2
  assert(dup2(ep_b, ep_a) == ep_a);
#else
  assert(close(ep_a) == 0);
  assert(open("/dev/null", O_RDONLY) == ep_a);
#endif
  emscripten_set_immediate(check, NULL);
  return 0;
}
