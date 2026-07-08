/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * dup(2) of an epoll fd yields another reference to the SAME epoll instance
 * (Linux eventpoll semantics): registrations, the ready list, and the persistent
 * readiness callback are all shared across every fd. This mirrors tokio's
 * single-threaded reactor, which arms emscripten_epoll_set_callback on one fd
 * and drives epoll_ctl(ADD) through a dup of it.
 *   - A registration added via the dup must be delivered to a callback armed on
 *     the original fd.
 *   - Closing one dup must NOT tear the instance down while another fd is open;
 *     only the last close reclaims it.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep_a, ep_b, rfd, wfd;
static int fires;

static void on_ready(int epfd, struct epoll_event* events, int nready, void* ud) {
  assert(epfd == ep_a);
  assert(nready == 1);
  assert(events[0].events & EPOLLIN);
  assert(events[0].data.u32 == 0x1234);
  fires++;

  char b[1];
  assert(read(rfd, b, 1) == 1);
  assert(emscripten_epoll_set_callback(ep_a, 4, NULL, NULL) == 0);
  printf("done\n");
}

int main(void) {
  ep_a = epoll_create1(0);

  // Arm the persistent callback on the original fd.
  assert(emscripten_epoll_set_callback(ep_a, 4, on_ready, NULL) == 0);

  // dup: a second fd to the SAME epoll instance (like tokio's registry handle).
  ep_b = dup(ep_a);
  assert(ep_b >= 0 && ep_b != ep_a);

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
  return 0;
}
