/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A listener delivery is a microtask: it runs once the call that made the set
 * ready has returned, and before the host task that call ran in ends, so two
 * fds made ready from two host tasks are each delivered in their own task.
 * A set that stays ready is redelivered from an immediate instead, as is a
 * wake raised by the callback itself, so neither can starve a timer.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

int ep;
int p[2][2];
int task = -1;
int delivered[2];
int spinning;
int spins;

void on_ready(int epfd, void* ud);

void on_timer(void* ud) {
  assert(spins > 1);
  assert(emscripten_epoll_listener_remove(ep, on_ready, NULL) == 0);
  printf("done\n");
  emscripten_runtime_keepalive_pop();
}

void on_ready(int epfd, void* ud) {
  struct epoll_event events[4];
  if (spinning) {
    assert(++spins < 10000 && "delivery starved the timer");
    // Wake the set again from under the callback, and leave it ready.
    assert(write(p[1][1], "x", 1) == 1);
    return;
  }
  // Only the running task's fd is ready: the other task's delivery is its own.
  assert(task >= 0 && !delivered[task]);
  assert(epoll_wait(epfd, events, 4, 0) == 1);
  assert(events[0].data.fd == p[task][0]);
  char b[1];
  assert(read(p[task][0], b, 1) == 1);
  delivered[task] = 1;
}

EMSCRIPTEN_KEEPALIVE void task_begin(int i) {
  task = i;
  assert(write(p[i][1], "x", 1) == 1);
  // Not under the frames of the write that made the set ready.
  assert(!delivered[i]);
}

EMSCRIPTEN_KEEPALIVE void task_end(int i) {
  assert(delivered[i] && "delivery did not run within the task");
  task = -1;
  if (i == 0) {
    return;
  }
  // A level fd that is never drained: a pipe write end is always writable.
  struct epoll_event ev = { .events = EPOLLOUT };
  ev.data.fd = p[0][1];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0][1], &ev) == 0);
  spinning = 1;
  emscripten_set_timeout(on_timer, 0, NULL);
}

EM_JS(void, run_host_tasks, (void), {
  for (let i = 0; i < 2; i++) {
    setTimeout(() => {
      _task_begin(i);
      // Queued after the wake, so it runs after the delivery, in this task.
      queueMicrotask(() => _task_end(i));
    });
  }
});

int main() {
  ep = epoll_create1(0);
  for (int i = 0; i < 2; i++) {
    assert(pipe(p[i]) == 0);
    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.fd = p[i][0];
    assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[i][0], &ev) == 0);
  }
  assert(emscripten_epoll_listener_add(ep, on_ready, NULL) == 0);
  run_host_tasks();
  emscripten_runtime_keepalive_push();
  return 0;
}
