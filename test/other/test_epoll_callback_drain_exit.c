/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A scheduled delivery holds the runtime until it runs. If the set is drained
 * synchronously before then (MODE_DRAIN: epoll_wait(..., 0) from main) or the
 * listener is removed (MODE_REMOVE), the delivery has nothing to do - but
 * releasing its hold may be what lets main's deferred exit proceed, so the
 * runtime must still exit: atexit prints "done", Module.onExit "exited", and
 * the process exits with main's status.
 *
 * Under PROXY_TO_PTHREAD the listener is owned by the proxied main thread. It
 * may see one spurious wakeup: the main thread's delivery can be dispatched
 * between the proxied write and the proxied drain, and its epoll_wait(0) then
 * collects nothing. Exits are explicit there: a proxied main whose keepalive
 * later reaches zero does not run exit()
 * (https://github.com/emscripten-core/emscripten/issues/27721).
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#define EXIT(rc) exit(rc)
#else
#define EXIT(rc) return rc
#endif

static int ep, rfd, wfd;

static void nothing_to_collect(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 0);
#ifndef __EMSCRIPTEN_PTHREADS__
  printf("delivered after drain\n");
  abort();
#endif
}

static void at_exit(void) {
  printf("done\n");
}

int main(void) {
  MAIN_THREAD_EM_ASM({ Module['onExit'] = () => out('exited'); });
  atexit(at_exit);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  ep = epoll_create1(0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);
  assert(emscripten_epoll_add_listener(ep, nothing_to_collect, 0) == 0);
  // Ready: a delivery is now scheduled and holds the runtime.
  assert(write(wfd, "x", 1) == 1);
#if MODE_REMOVE
  assert(emscripten_epoll_remove_listener(ep, nothing_to_collect, 0) == 0);
#else
  assert(epoll_wait(ep, &ev, 1, 0) == 1 && ev.data.fd == rfd);
  char b;
  assert(read(rfd, &b, 1) == 1);
#endif
  EXIT(7);
}
