/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_epoll_set_callback woken by real socket readiness (arriving UDP
 * datagrams) through the SOCKFS -> wait-queue bridge, with no blocking call and
 * no ASYNCIFY/JSPI. A single arm delivers repeatedly: each datagram is a
 * separate producer event that re-fires the persistent callback.
 */

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static int ep, rx, tx;
static struct sockaddr_in addr;
static int fires;

static void send_one(const char* msg) {
  assert(sendto(tx, msg, 4, 0, (struct sockaddr*)&addr, sizeof addr) == 4);
}

static void on_ready(void* ud) {
  struct epoll_event ev[4];
  assert(epoll_wait(ep, ev, 4, 0) == 1);
  assert(ev[0].events & EPOLLIN);
  assert(ev[0].data.fd == rx);
  char b[4];
  assert(recv(rx, b, 4, 0) == 4);
  fires++;

  if (fires == 1) {
    assert(memcmp(b, "one\0", 4) == 0);
    send_one("two");   // a second producer event re-fires the same arm
    return;
  }
  assert(fires == 2);
  assert(memcmp(b, "two\0", 4) == 0);
  printf("done\n");
  // Closing the watched fd makes the epoll terminal - nothing it watches can
  // become ready again - so the callback stops keeping the runtime alive and the
  // process exits (no explicit unregister needed).
  close(rx);
  close(tx);
}

int main(void) {
  ep = epoll_create1(0);
  rx = socket(AF_INET, SOCK_DGRAM, 0);
  tx = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET; addr.sin_port = htons(0);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(rx, (struct sockaddr*)&addr, sizeof addr) == 0);
  socklen_t l = sizeof addr;
  assert(getsockname(rx, (struct sockaddr*)&addr, &l) == 0);

  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rx;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rx, &ev) == 0);

  // Arm once (no ASYNCIFY), then send the first datagram; it arrives after we
  // return and wakes the callback. The callback drives the second send itself.
  assert(emscripten_epoll_set_callback(ep, on_ready, 0) == 0);
  send_one("one");
  return 0;
}
