/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A readiness callback on an outer epoll that nests an inner one. A single leaf
 * edge must propagate two levels - leaf -> inner epoll's wait-queue -> outer
 * epoll's registration -> outer epoll's wait-queue -> the callback - and surface
 * as readiness on the inner epoll's fd, with no blocking and no ASYNCIFY/JSPI.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int epA, epB, rfd, wfd;

static void writer(void* arg) { assert(write(wfd, "x", 1) == 1); }

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(epA, ev, 4, 0) == 1);
  assert(ev[0].data.fd == epB); // the inner epoll, surfaced through nesting
  assert(ev[0].events & EPOLLIN);
  char b[1];
  assert(read(rfd, b, 1) == 1); // drain the leaf
  assert(emscripten_epoll_set_callback(epA, NULL, NULL) == 0);
  printf("done\n");
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
  assert(epoll_ctl(epB, EPOLL_CTL_ADD, rfd, &ev) == 0); // leaf in the inner epoll
  ev.data.fd = epB;
  assert(epoll_ctl(epA, EPOLL_CTL_ADD, epB, &ev) == 0); // inner epoll in the outer

  // Arm the callback on the outer epoll, then write after we return: the leaf
  // edge wakes the callback through both levels with no stack switch.
  assert(emscripten_epoll_set_callback(epA, on_ready, 0) == 0);
  emscripten_async_call(writer, NULL, 0);
  return 0;
}
