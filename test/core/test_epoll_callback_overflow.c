/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_epoll_set_callback drain across ticks: the callback fires while the
 * poll queue has ready events, so a callback that collects only one per tick
 * (epoll_wait maxevents=1) is re-triggered until the queue drains - there is no
 * app loop to re-call it. Three always-readable fds are all delivered (each
 * exactly once, round-robin) from a single arm and a single set of writes, with
 * no further producer events.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep;
static int rfd[3];
static int fires;
static int seen[3];

static int index_of(int fd) {
  for (int i = 0; i < 3; i++) if (rfd[i] == fd) return i;
  return -1;
}

static void on_ready(void* ud) {
  struct epoll_event ev[1];
  assert(epoll_wait(ep, ev, 1, 0) == 1); // collect one per tick
  int i = index_of(ev[0].data.fd);
  assert(i >= 0 && !seen[i]); // each fd delivered exactly once (no starvation)
  seen[i] = 1;
  char b[1];
  assert(read(rfd[i], b, 1) == 1); // drain so it is no longer ready

  if (++fires == 3) {
    assert(emscripten_epoll_set_callback(ep, NULL, NULL) == 0);
    printf("done\n");
  }
}

int main(void) {
  ep = epoll_create1(0);
  for (int i = 0; i < 3; i++) {
    int p[2];
    assert(pipe(p) == 0);
    rfd[i] = p[0];
    assert(write(p[1], "x", 1) == 1); // read end readable (level)
    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.fd = rfd[i];
    assert(epoll_ctl(ep, EPOLL_CTL_ADD, rfd[i], &ev) == 0);
  }

  // One arm, three ready fds, and a callback that collects one per tick: it must
  // be re-triggered to deliver all three (one per tick), not just the first.
  assert(emscripten_epoll_set_callback(ep, on_ready, 0) == 0);
  return 0;
}
