/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_force_exit with a listener registered on an armed socket: FS.quit
 * closes the epoll fd on the way out, removing the listener, and the teardown
 * wake that raises delivers nothing and holds nothing.
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
  // The listener is armed and the socket left open when the forced exit runs:
  // FS.quit closes the epoll and removes the listener on the way out.
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
  emscripten_async_call(quit, NULL, 0);
  return 0;
}
