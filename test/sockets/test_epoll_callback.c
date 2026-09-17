/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * An epoll listener callback woken by datagrams arriving on a real socket, with
 * no ASYNCIFY/JSPI. The datagram lands from the host after main returns, so
 * the program holds the runtime itself with emscripten_runtime_keepalive_push()
 * and pops from the callback once done. With MODE_UNREF nothing is held: main
 * returning exits the runtime at once and the callback never runs, even though
 * a datagram is in flight to an armed socket.
 */

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <emscripten.h>
#include <emscripten/epoll.h>
#include <emscripten/eventloop.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int ep, rx, tx;
struct sockaddr_in addr;
int fires;

void send_one(const char* msg) {
  assert(sendto(tx, msg, 4, 0, (struct sockaddr*)&addr, sizeof addr) == 4);
}

void on_ready(void* ud) {
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
  close(rx);
  close(tx);
  // Done: release the hold taken in main so the runtime exits.
  emscripten_runtime_keepalive_pop();
#ifdef __EMSCRIPTEN_PTHREADS__
  // Under PROXY_TO_PTHREAD releasing the last hold on the worker exits only the
  // thread, not the process; exit explicitly.
  exit(0);
#endif
}

void at_exit(void) {
#ifdef MODE_UNREF
  assert(fires == 0);
#else
  assert(fires == 2);
#endif
  printf("done\n");
}

int main(void) {
  atexit(at_exit);
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
  assert(emscripten_epoll_add_listener(ep, on_ready, 0) == 0);
  send_one("one");
#ifndef MODE_UNREF
  emscripten_runtime_keepalive_push();
#endif
  return 0;
}
