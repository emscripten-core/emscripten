/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Pins the documented level-triggered callback behaviour: an fd that is
 * structurally always ready (here a pipe write end, always EPOLLOUT) re-fires
 * the callback on every event-loop tick. The runtime drives that loop, so such
 * an fd would spin indefinitely - the contract is that the app uses EPOLLET or
 * unregisters. This test unregisters after a few deliveries so it terminates.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, fires;

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1);
  assert(ev[0].events & EPOLLOUT);
  if (++fires == 3) { // re-fired every tick despite no new event and no drain
    assert(emscripten_epoll_set_callback(ep, NULL, NULL) == 0);
    printf("done\n");
  }
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  struct epoll_event ev = { .events = EPOLLOUT }; // level; a write end is always writable
  ev.data.fd = p[1];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &ev) == 0);

  assert(emscripten_epoll_set_callback(ep, on_ready, 0) == 0);
  return 0;
}
