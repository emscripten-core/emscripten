/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Listener registration identity: a listener is keyed by (callback, userdata),
 * so the same callback registers once per userdata, re-adding a registered pair
 * is EEXIST, and emscripten_epoll_remove_listener removes by pair (ENOENT when
 * absent, EBADF on a non-epoll fd).
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int ep, rfd, wfd;
int fired[3];

void on_ready(void* ud) {
  // Both registrations of on_ready are signalled while the pipe is ready; each
  // takes one of the two bytes.
  fired[(long)ud]++;
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1);
  char b[1];
  assert(read(rfd, b, 1) == 1);
  if (fired[1] && fired[2]) {
    assert(fired[1] == 1 && fired[2] == 1);
    // Remove both, then make the set ready again to prove no further delivery
    // happens.
    assert(emscripten_epoll_remove_listener(ep, on_ready, (void*)1) == 0);
    assert(emscripten_epoll_remove_listener(ep, on_ready, (void*)1) == ENOENT);
    assert(emscripten_epoll_remove_listener(ep, on_ready, (void*)2) == 0);
    assert(write(wfd, "x", 1) == 1);
    printf("done\n");
  }
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rfd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd, &ev) == 0);

  // A non-epoll fd is rejected with a positive EBADF.
  assert(emscripten_epoll_add_listener(rfd, on_ready, 0) == EBADF);
  assert(emscripten_epoll_remove_listener(rfd, on_ready, 0) == EBADF);
  // Removing a never-added listener is ENOENT.
  assert(emscripten_epoll_remove_listener(ep, on_ready, 0) == ENOENT);

  // The same callback with two userdatas is two listeners; the same pair twice
  // is one.
  assert(emscripten_epoll_add_listener(ep, on_ready, (void*)1) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, (void*)2) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, (void*)1) == EEXIST);
  assert(write(wfd, "xy", 2) == 2); // delivered on the next tick to both
  return 0;
}
