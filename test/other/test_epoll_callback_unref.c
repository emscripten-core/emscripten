/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A listener is an unref'd handle: it never keeps the runtime (or, with
 * pthreads, its registering thread) alive. Without a hold, main returning
 * exits the runtime at once with main's status and the callback never runs.
 * With MODE_HOLD the program holds the runtime itself with
 * emscripten_runtime_keepalive_push() before returning; the delivery then runs
 * on the registering thread, and the pop from the callback lets the runtime
 * exit, with atexit and onExit both firing.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int ep, rfd, wfd, fires;

void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1 && (ev[0].events & EPOLLIN));
  char b;
  assert(read(rfd, &b, 1) == 1);
  fires++;
  emscripten_runtime_keepalive_pop();
#ifdef __EMSCRIPTEN_PTHREADS__
  // Under PROXY_TO_PTHREAD releasing the last hold on the worker exits only the
  // thread, not the process; exit explicitly.
  exit(0);
#endif
}

void writer(void* arg) { assert(write(wfd, "x", 1) == 1); }

void at_exit(void) {
#ifdef MODE_HOLD
  assert(fires == 1);
#else
  assert(fires == 0);
#endif
  printf("done\n");
}

int main(void) {
  MAIN_THREAD_EM_ASM({ Module['onExit'] = (status) => out('exited ' + status); });
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
#ifdef MODE_HOLD
  emscripten_runtime_keepalive_push();
  emscripten_set_timeout(writer, 0, NULL);
  return 0;
#else
  // Armed and listening, nothing held: exit now, callback never runs.
  return 3;
#endif
}
