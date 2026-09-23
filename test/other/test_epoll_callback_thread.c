/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Listeners are main-thread only: adding or removing one from another thread
 * (here the proxied main under PROXY_TO_PTHREAD) fails with ENOTSUP.
 */

#include <sys/epoll.h>
#include <emscripten/epoll.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

void on_ready(void* ud) {}

int main(void) {
  int ep = epoll_create1(0);
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == ENOTSUP);
  assert(emscripten_epoll_remove_listener(ep, on_ready, 0) == ENOTSUP);
  printf("done\n");
  return 0;
}
