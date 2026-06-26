/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Exercises the epoll syscall surface (epoll_create1/epoll_ctl/epoll_wait):
 * the interest set, the ADD/MOD/DEL ops with their error returns, readiness
 * derivation over a pipe, and that the opaque `data` is echoed back.
 */

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int main(void) {
  int ep = epoll_create1(0);
  assert(ep >= 0);

  int p[2];
  assert(pipe(p) == 0);

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == -1 && errno == EEXIST);

  struct epoll_event out[4];
  // Nothing written yet: the read end is not readable.
  assert(epoll_wait(ep, out, 4, 0) == 0);

  // Make the read end readable.
  assert(write(p[1], "x", 1) == 1);
  assert(epoll_wait(ep, out, 4, 0) == 1);
  assert(out[0].events & EPOLLIN);
  assert(out[0].data.fd == p[0]);

  // MOD to a condition that is not satisfied (writable on the read end).
  ev.events = EPOLLOUT;
  assert(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &ev) == 0);
  assert(epoll_wait(ep, out, 4, 0) == 0);

  // DEL, and DEL again -> ENOENT.
  assert(epoll_ctl(ep, EPOLL_CTL_DEL, p[0], &ev) == 0);
  assert(epoll_ctl(ep, EPOLL_CTL_DEL, p[0], &ev) == -1 && errno == ENOENT);
  assert(epoll_wait(ep, out, 4, 0) == 0);

  // Bad epoll fd and bad target fd.
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, 9999, &ev) == -1 && errno == EBADF);
  assert(epoll_ctl(9999, EPOLL_CTL_ADD, p[0], &ev) == -1 && errno == EBADF);

  // Bad op.
  ev.events = EPOLLIN;
  assert(epoll_ctl(ep, 999, p[0], &ev) == -1 && errno == EINVAL);

  // An epoll cannot watch itself.
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, ep, &ev) == -1 && errno == EINVAL);

  // Regular files are not epoll-capable (no readiness derivation -> EPERM).
  int rf = open("/tmp/epoll_regular", O_CREAT | O_RDWR, 0600);
  assert(rf >= 0);
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rf, &ev) == -1 && errno == EPERM);
  close(rf);

  // maxevents must be positive.
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);
  assert(epoll_wait(ep, out, 0, 0) == -1 && errno == EINVAL);
  assert(epoll_wait(ep, out, -1, 0) == -1 && errno == EINVAL);

  close(ep);
  close(p[0]);
  close(p[1]);
  printf("EPOLL PASS\n");
  return 0;
}
