/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Delivery scheduling. A burst of readiness edges before the notifying call
 * returns (here many small pipe writes; on a socket, a run of incoming chunks)
 * coalesces onto one delivery, which runs as a microtask of the host task that
 * made the set ready. A set left ready, or woken by the callback itself, is
 * redelivered from an immediate, so it cannot starve a timer.
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
    let orig = globalThis[fn];
    globalThis[fn] = (...args) => {
      globalThis.deliveries++;
      return orig(...args);
    };
  }
});

EM_JS(int, deliveries_scheduled, (void), {
  return globalThis.deliveries;
});

int ep;
int p[2][2];
int task = -1;
int delivered[2];
int spinning;
int spins;

void on_ready(int epfd, void* ud) {
  if (spinning) {
    assert(++spins < 10000 && "delivery starved the timer");
    // Wake the set from under the callback, and leave it ready.
    assert(write(p[1][1], "x", 1) == 1);
    return;
  }
  // Only the running task's fd is ready.
  struct epoll_event ev[2];
  assert(epoll_wait(epfd, ev, 2, 0) == 1);
  assert(ev[0].data.fd == p[task][0]);
  char buf[WRITES];
  assert(read(p[task][0], buf, sizeof(buf)) == WRITES);
  delivered[task] = 1;
}

void on_timer(void* ud) {
  assert(spins > 1);
  assert(emscripten_epoll_listener_remove(ep, on_ready, 0) == 0);
  printf("done\n");
  emscripten_runtime_keepalive_pop();
}

EMSCRIPTEN_KEEPALIVE void task_begin(int i) {
  task = i;
  for (int n = 0; n < WRITES; n++) {
    assert(write(p[i][1], "x", 1) == 1);
  }
  // Not under the frames of the writes.
  assert(!delivered[i]);
}

EMSCRIPTEN_KEEPALIVE void task_end(int i) {
  assert(delivered[i] && "delivery did not run within the task");
  // One delivery for registering the listener, and one per task.
  assert(deliveries_scheduled() == i + 2);
  if (i == 0) {
    return;
  }
  // A level fd that is never drained: a pipe write end is always writable.
  struct epoll_event ev = { .events = EPOLLOUT };
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0][1], &ev) == 0);
  spinning = 1;
  emscripten_set_timeout(on_timer, 0, NULL);
}

EM_JS(void, run_host_tasks, (void), {
  for (let i = 0; i < 2; i++) {
    setTimeout(() => {
      _task_begin(i);
      // Queued after the wake, so it follows the delivery within this task.
      Promise.resolve().then(() => _task_end(i));
    });
  }
});

int main() {
  count_deliveries();
  ep = epoll_create1(0);
  for (int i = 0; i < 2; i++) {
    assert(pipe(p[i]) == 0);
    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.fd = p[i][0];
    assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[i][0], &ev) == 0);
  }
  assert(emscripten_epoll_listener_add(ep, on_ready, 0) == 0);
  run_host_tasks();
  emscripten_runtime_keepalive_push();
  return 0;
}
