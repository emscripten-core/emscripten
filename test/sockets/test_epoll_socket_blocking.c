/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A *blocking* epoll_wait() on a real socket is woken by a datagram that arrives
 * *after* the wait has already blocked. The datagram is sent on a delay (from
 * another thread under -pthread, or a timer under JSPI) so epoll_wait() must
 * suspend - the proxied worker under PROXY_TO_PTHREAD, or the calling stack
 * under JSPI - and be woken through the unified readiness wait-queue, the same
 * SOCKFS.emit bridge poll() rides.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif

static int rx = -1, tx = -1;
static struct sockaddr_in addr;

static void send_ping(void* arg) {
  assert(sendto(tx, "ping", 4, 0, (struct sockaddr*)&addr, sizeof(addr)) == 4);
}

#ifdef __EMSCRIPTEN_PTHREADS__
static void* sender(void* arg) {
  usleep(100000); // let epoll_wait() block first
  send_ping(NULL);
  return NULL;
}
#endif

int main(void) {
  rx = socket(AF_INET, SOCK_DGRAM, 0);
  tx = socket(AF_INET, SOCK_DGRAM, 0);
  assert(rx >= 0 && tx >= 0);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(rx, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  socklen_t l = sizeof(addr);
  assert(getsockname(rx, (struct sockaddr*)&addr, &l) == 0);

  int ep = epoll_create1(0);
  assert(ep >= 0);
  struct epoll_event ev = { .events = EPOLLIN };
  ev.data.fd = rx;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rx, &ev) == 0);

  // Arrange the datagram to arrive only after epoll_wait() is already blocking,
  // so it can only complete by being woken - not by the initial derivation.
#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t t;
  assert(pthread_create(&t, NULL, sender, NULL) == 0);
#else
  emscripten_async_call(send_ping, NULL, 100);
#endif

  struct epoll_event out[4];
  int n = epoll_wait(ep, out, 4, -1); // blocks; only the arrival can wake it
  assert(n == 1 && (out[0].events & EPOLLIN));
  assert(out[0].data.fd == rx);

  char buf[4];
  assert(recv(rx, buf, sizeof(buf), 0) == 4 && memcmp(buf, "ping", 4) == 0);

  close(ep);
  close(rx);
  close(tx);
  printf("EPOLL SOCKET BLOCKING PASS\n");
  return 0;
}
