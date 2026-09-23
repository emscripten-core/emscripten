/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A fatal error escaping the callback surfaces as an uncaught exception from
 * the delivery's macrotask, as it would from any other event loop callback, and
 * not as an unhandled promise rejection.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void on_ready(void* ud) {
  printf("aborting\n");
  abort();
}

int main(void) {
  EM_ASM({
    process.on('unhandledRejection', () => {
      out('unhandled rejection');
      process.exit(0);
    });
  });
  int p[2];
  assert(pipe(p) == 0);
  int ep = epoll_create1(0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = p[0];
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
  assert(write(p[1], "x", 1) == 1);
  return 0;
}
