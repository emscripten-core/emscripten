/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A blocking epoll_wait() that suspends the wasm stack (ASYNCIFY/JSPI) and is
 * woken by a pipe write scheduled to run only after it has blocked.
 */

#include <sys/epoll.h>
#include <emscripten.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int wfd;
static void writer(void* arg) { assert(write(wfd, "x", 1) == 1); }

int main(void) {
  int ep = epoll_create1(0);
  int p[2];
  assert(pipe(p) == 0);
  wfd = p[1];
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.u32 = 0xabcd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);

  // The write happens only after epoll_wait suspends.
  emscripten_async_call(writer, NULL, 0);

  struct epoll_event out[4];
  int n = epoll_wait(ep, out, 4, -1);
  assert(n == 1);
  assert(out[0].events & EPOLLIN);
  assert(out[0].data.u32 == 0xabcd);
  printf("done\n");
  return 0;
}
