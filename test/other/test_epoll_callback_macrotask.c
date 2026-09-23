/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A listener delivery is a macrotask, ordered after every microtask queued
 * before it runs, however late. Some hosts drain the microtask queue
 * synchronously inside unrelated calls (a builtin module load), so a microtask
 * delivery could run the callback re-entrantly under the frames of whatever
 * wasm call happened to be executing; a macrotask never can.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, rfd, wfd;
static int microtask_ran;

EM_JS(void, queue_microtask_marker, (int* flag), {
  queueMicrotask(() => { HEAP32[flag >> 2] = 1; });
});

static void on_ready(void* ud) {
  // Queued after the set became ready, from the frame that made it ready.
  assert(microtask_ran && "delivery ran before an earlier-queued microtask");
  struct epoll_event events[1];
  assert(epoll_wait(ep, events, 1, 0) == 1);
  char b[1];
  assert(read(rfd, b, 1) == 1);
  assert(emscripten_epoll_remove_listener(ep, on_ready, NULL) == 0);
  printf("done\n");
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN };
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, NULL) == 0);

  // Readiness schedules the delivery; a microtask queued afterwards must still
  // run first.
  assert(write(wfd, "x", 1) == 1);
  queue_microtask_marker(&microtask_ran);
  return 0;
}
