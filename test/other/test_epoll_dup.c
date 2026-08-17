/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * dup(2) of an epoll fd yields another reference to the SAME epoll instance
 * (Linux eventpoll semantics): registrations and the ready list are shared
 * across every fd.
 *   - A registration added via the dup must be visible to a wait on the
 *     original fd.
 *   - Closing one dup must NOT tear the instance down while another fd is open;
 *     only the last close reclaims it.
 */

#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
  int ep_a = epoll_create1(0);
  assert(ep_a >= 0);

  // dup: a second fd to the SAME epoll instance.
  int ep_b = dup(ep_a);
  assert(ep_b >= 0 && ep_b != ep_a);

  int p[2];
  assert(pipe(p) == 0);
  int rfd = p[0], wfd = p[1];

  // Register through the dup. This must be visible to waits on ep_a, since
  // both fds share one epoll instance.
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.u32 = 0x1234;
  assert(epoll_ctl(ep_b, EPOLL_CTL_ADD, rfd, &ev) == 0);

  // Closing one dup must not tear the instance down: the registration added
  // via ep_b stays live and waits on ep_a keep working.
  assert(close(ep_b) == 0);

  // Make rfd readable. The edge must be reported through ep_a.
  assert(write(wfd, "x", 1) == 1);
  struct epoll_event events[4];
  assert(epoll_wait(ep_a, events, 4, 0) == 1);
  assert(events[0].events & EPOLLIN);
  assert(events[0].data.u32 == 0x1234);

  // Level-triggered: still ready until drained.
  assert(epoll_wait(ep_a, events, 4, 0) == 1);
  char b[1];
  assert(read(rfd, b, 1) == 1);
  assert(epoll_wait(ep_a, events, 4, 0) == 0);

  // The last close reclaims the instance.
  assert(close(ep_a) == 0);
  assert(epoll_wait(ep_a, events, 4, 0) == -1 && errno == EBADF);

  printf("done\n");
  return 0;
}
