/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A listener delivery is a macrotask: it runs once the call that made the set
 * ready and the current microtask checkpoint have completed, never under the
 * notifying call's frames.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, rfd, wfd;
static int write_returned;
static int microtask_ran;

EM_JS(void, queue_microtask_marker, (int* flag), {
  queueMicrotask(() => { HEAP32[flag >> 2] = 1; });
});

static void on_ready(void* ud) {
  // Not under the frames of the write that made the set ready.
  assert(write_returned && "delivery ran inside the call that made the set ready");
  // After a microtask queued after that write: delivery is a macrotask.
  assert(microtask_ran && "delivery ran before a later-queued microtask");
  struct epoll_event events[1];
  assert(epoll_wait(ep, events, 1, 0) == 1);
  char b[1];
  assert(read(rfd, b, 1) == 1);
  assert(emscripten_epoll_listener_remove(ep, on_ready, NULL) == 0);
  printf("done\n");
  emscripten_runtime_keepalive_pop();
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN };
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);
  assert(emscripten_epoll_listener_add(ep, on_ready, NULL) == 0);

  // Readiness schedules the delivery after the microtask queued next.
  assert(write(wfd, "x", 1) == 1);
  write_returned = 1;
  queue_microtask_marker(&microtask_ran);
  emscripten_runtime_keepalive_push();
  return 0;
}
