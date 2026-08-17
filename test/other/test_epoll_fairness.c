/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Round-robin fairness: with more ready fds than maxevents, successive waits
 * must rotate. A delivered level-triggered fd goes to the back of the ready
 * list, and the unprocessed remainder is serviced first on the next call, so no
 * fd starves. With three always-readable fds and maxevents=1, the reported fd
 * cycles a, b, c, a, b, c.
 */

#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
  int ep = epoll_create1(0);
  int rfd[3];
  for (int i = 0; i < 3; i++) {
    int p[2];
    assert(pipe(p) == 0);
    rfd[i] = p[0];
    assert(write(p[1], "x", 1) == 1); // read end is now readable (level), never drained
    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.fd = rfd[i];
    assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd[i], &ev) == 0);
  }

  int expect[6] = { rfd[0], rfd[1], rfd[2], rfd[0], rfd[1], rfd[2] };
  struct epoll_event out;
  for (int i = 0; i < 6; i++) {
    assert(epoll_wait(ep, &out, 1, 0) == 1);
    assert(out.data.fd == expect[i]);
  }

  printf("done\n");
  return 0;
}
