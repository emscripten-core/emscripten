/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A burst of readiness edges before the notifying call returns (here many
 * small pipe writes; on a socket, a run of incoming chunks) coalesces onto one
 * pending delivery rather than one per edge.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#define WRITES 1000

EM_JS(void, count_deliveries, (void), {
  globalThis.deliveries = 0;
  for (let fn of ['queueMicrotask', 'setImmediate']) {
    var orig = globalThis[fn];
    globalThis[fn] = (...args) => {
      globalThis.deliveries++;
      return orig(...args);
    };
  }
});

EM_JS(int, deliveries_scheduled, (void), {
  return globalThis.deliveries;
});

int fires;

void on_ready(int ep, void* ud) {
  struct epoll_event ev;
  assert(epoll_wait(ep, &ev, 1, 0) == 1);
  assert(ev.events & EPOLLIN);
  char buf[WRITES];
  assert(read(ev.data.fd, buf, sizeof(buf)) == WRITES);
  fires++;
  // Registering the listener and every write each wake the epoll; all of those
  // wakes happened before this first turn, so one delivery covers them.
  int scheduled = deliveries_scheduled();
  printf("fires=%d deliveries=%d\n", fires, scheduled);
  assert(scheduled == 1);
  assert(emscripten_epoll_listener_remove(ep, on_ready, 0) == 0);
  printf("done\n");
  emscripten_runtime_keepalive_pop();
}

int main() {
  count_deliveries();
  int ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);
  assert(emscripten_epoll_listener_add(ep, on_ready, 0) == 0);
  for (int i = 0; i < WRITES; i++) {
    assert(write(p[1], "x", 1) == 1);
  }
  emscripten_runtime_keepalive_push();
  return 0;
}
