/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Only host-backed registrations (sockets) hold the runtime alive. A pipe can
 * only be written by wasm, so it can never fire from the host: a listener over
 * an armed pipe alone must not keep the runtime alive once main returns (the
 * process exits, running atexit), even though a pipe write scheduled by other
 * live work (a timer) still delivers.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int ep, rfd, wfd, fires;

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1 && (ev[0].events & EPOLLIN));
  char b[1];
  assert(read(rfd, b, 1) == 1);
  fires++;
}

static void writer(void* arg) { assert(write(wfd, "x", 1) == 1); }

static void at_exit(void) {
  // Delivered once (the timer-driven write), then exited with the pipe still
  // armed and the listener still registered.
  assert(fires == 1);
  printf("done\n");
}

int main(void) {
  atexit(at_exit);
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
  emscripten_async_call(writer, NULL, 0);
  return 0;
}
