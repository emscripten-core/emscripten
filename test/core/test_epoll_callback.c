/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_epoll_set_callback: a persistent, non-blocking, non-suspending epoll
 * readiness callback (no ASYNCIFY/JSPI). The callback receives only its userdata
 * and collects the ready events itself with a zero-timeout epoll_wait. A single
 * arm delivers repeatedly. The arming itself is an event source - matching Linux,
 * where the set becomes ready with no producer wakeup to follow:
 *   - EPOLL_CTL_ADD of an already-readable fd signals it.
 *   - EPOLL_CTL_MOD re-arming a still-readable EPOLLONESHOT fd signals it again.
 * Clearing the interest (NULL callback) stops delivery and lets the runtime exit.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep, rfd, wfd;
static int fires;

static void arm_rfd(int op) {
  struct epoll_event ev = { .events = EPOLLIN | EPOLLONESHOT };
  ev.data.u32 = 0x1234;
  assert(epoll_ctl(ep, op, rfd, &ev) == 0);
}

static void on_ready(void* ud) {
  assert((long)ud == 42);
  struct epoll_event events[4];
  int nready = epoll_wait(ep, events, 4, 0);
  assert(nready == 1);
  assert(events[0].events & EPOLLIN);
  assert(events[0].data.u32 == 0x1234);
  fires++;

  if (fires == 1) {
    // EPOLLONESHOT disabled the registration on this delivery, but the byte is
    // still in the pipe (level-readable). Re-arm with MOD WITHOUT draining: with
    // no producer event to follow, only the MOD poke can re-evaluate readiness.
    arm_rfd(EPOLL_CTL_MOD);
    return;
  }

  assert(fires == 2);
  // Drain, clear the interest, then make the set ready again: with the callback
  // cleared there is nothing left to fire, and the runtime exits cleanly.
  char b[1];
  assert(read(rfd, b, 1) == 1);
  assert(emscripten_epoll_set_callback(ep, NULL, NULL) == 0);
  assert(write(wfd, "x", 1) == 1);
  arm_rfd(EPOLL_CTL_MOD);
  printf("done\n");
}

int main(void) {
  ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  rfd = p[0];
  wfd = p[1];

  // Arm the persistent callback on an empty set: nothing ready, no fire.
  assert(emscripten_epoll_set_callback(ep, on_ready, (void*)42) == 0);

  // Make rfd readable, then ADD it. The fd is already ready with no producer
  // wakeup to come, so the ADD itself must trigger the first delivery.
  assert(write(wfd, "x", 1) == 1);
  arm_rfd(EPOLL_CTL_ADD);
  return 0;
}
