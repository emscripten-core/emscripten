/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Wakes raised while the runtime is exiting must hold nothing: exitRuntime's
 * FS.quit closes every fd still open, in fd order, and a hold taken there
 * outlives the exit, leaving keepRuntimeAlive() set when _proc_exit runs, so
 * Module.onExit is skipped. The pipe is created before the epoll so its ends
 * close first: the read end's POLLNVAL, then the write end's close reporting
 * POLLHUP on the still-armed registration - a readiness-shaped wake. The
 * runtime exits (atexit prints "done") and the exit completes (onExit prints
 * "exited").
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int ep, rfd;

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1 && ev[0].data.fd == rfd);
  char b;
  assert(read(rfd, &b, 1) == 1);
}

static void at_exit(void) {
  printf("done\n");
}

int main(void) {
  EM_ASM({ Module['onExit'] = () => out('exited'); });
  atexit(at_exit);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  ep = epoll_create1(0);

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
  // One pending delivery is held and drained; then, with the pipe and epoll
  // still open, nothing holds the runtime and main's return exits it.
  // Neither end is closed here: FS.quit closes them, pipe ends first.
  assert(write(p[1], "x", 1) == 1);
  return 0;
}
