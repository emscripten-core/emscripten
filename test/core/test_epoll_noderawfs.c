/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * poll()/epoll under -sNODERAWFS, where regular-file streams are backed by
 * node's fs and carry no stream_ops. The readiness layer must treat such a
 * stream as a plain always-ready file (not dereference a missing poll handler):
 * poll reports POLLIN|POLLOUT, epoll_ctl rejects it with EPERM, and a PIPEFS
 * pipe (still a real stream_ops-bearing stream under NODERAWFS) works normally.
 */

#include <sys/epoll.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int main(void) {
  int rf = open("epoll_noderawfs.tmp", O_CREAT | O_RDWR, 0600);
  assert(rf >= 0);

  // A regular file with no poll handler is always readable+writable, and does
  // not crash the derivation.
  struct pollfd pf = { .fd = rf, .events = POLLIN | POLLOUT };
  assert(poll(&pf, 1, 0) == 1);
  assert(pf.revents == (POLLIN | POLLOUT));

  // ...and is not epoll-capable.
  int ep = epoll_create1(0);
  assert(ep >= 0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rf;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rf, &ev) == -1 && errno == EPERM);

  // A pipe still comes from PIPEFS under NODERAWFS, so epoll works on it.
  int p[2];
  assert(pipe(p) == 0);
  ev.events = EPOLLIN;
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);
  struct epoll_event out[4];
  assert(epoll_wait(ep, out, 4, 0) == 0);
  assert(write(p[1], "x", 1) == 1);
  assert(epoll_wait(ep, out, 4, 0) == 1);
  assert(out[0].events & EPOLLIN);
  assert(out[0].data.fd == p[0]);

  close(ep);
  close(p[0]);
  close(p[1]);
  close(rf);
  unlink("epoll_noderawfs.tmp");
  printf("EPOLL NODERAWFS PASS\n");
  return 0;
}
