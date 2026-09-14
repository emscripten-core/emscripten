/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_force_exit while a listener holds the runtime: the forced exit
 * forfeits every keepalive hold before exitRuntime, whose FS.quit then closes the
 * epoll fd and releases the listener's (already forfeited) hold. That release
 * must not underflow the keepalive counter (which asserts).
 */

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int ep, rx;

static void on_ready(void* ud) {
  assert(0 && "nothing ever connects");
}

static void quit(void* arg) {
  printf("done\n");
  emscripten_force_exit(0);
}

int main(void) {
  ep = epoll_create1(0);
  rx = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(rx, (struct sockaddr*)&addr, sizeof addr) == 0);
  assert(listen(rx, 1) == 0); // readable only on a pending connection

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rx;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rx, &ev) == 0);
  // A socket registration holds the runtime open; the listener is armed and the
  // socket left open when the forced exit runs.
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
  emscripten_async_call(quit, NULL, 0);
  return 0;
}
