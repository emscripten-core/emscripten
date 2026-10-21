/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Duplicates share epoll interests and readiness, but own their listeners.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int ep_a, ep_b, rfd, wfd;
int fires;

void on_ready(int epfd, void* ud) {
  assert(epfd == ep_a);
  struct epoll_event events[4];
  assert(epoll_wait(epfd, events, 4, 0) == 1);
  assert(events[0].events & EPOLLIN);
  assert(events[0].data.u32 == 0x1234);
  fires++;

  char b[1];
  assert(read(rfd, b, 1) == 1);
  assert(emscripten_epoll_listener_remove(epfd, on_ready, NULL) == 0);
  printf("done\n");
  emscripten_runtime_keepalive_pop();
}

int main() {
  ep_a = epoll_create1(0);

  // Arm the persistent callback on the original fd.
  assert(emscripten_epoll_listener_add(ep_a, on_ready, NULL) == 0);

  // dup: a second fd to the SAME epoll instance (like tokio's registry handle).
  ep_b = dup(ep_a);
  assert(ep_b >= 0 && ep_b != ep_a);
  assert(emscripten_epoll_listener_remove(ep_b, on_ready, NULL) == ENOENT);
  assert(emscripten_epoll_listener_add(ep_b, on_ready, NULL) == 0);
  assert(emscripten_epoll_listener_remove(ep_b, on_ready, NULL) == 0);
  assert(emscripten_epoll_listener_remove(ep_b, on_ready, NULL) == ENOENT);
  assert(emscripten_epoll_listener_add(ep_a, on_ready, NULL) == EEXIST);

  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];

  // Register through the dup. This must be visible to the callback armed on
  // ep_a, since both fds share one epoll instance.
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.u32 = 0x1234;
  assert(epoll_ctl(ep_b, EPOLL_CTL_ADD, rfd, &ev) == 0);

  // Closing one dup must not tear the instance down: the registration added via
  // ep_b stays live and the callback on ep_a keeps working.
  assert(close(ep_b) == 0);

  // Make rfd readable. The edge must reach ep_a's callback.
  assert(write(wfd, "x", 1) == 1);
  emscripten_runtime_keepalive_push();
  return 0;
}
