/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A *blocking* poll() on a real socket is woken by data that arrives *after* the
 * poll has already blocked. The datagram is sent on a delay (from another thread
 * under -pthread, or a timer under JSPI) so poll() must suspend - the calling
 * stack under JSPI, or the proxied worker under PROXY_TO_PTHREAD - and be woken
 * by the arrival through the unified readiness wait-queue. Sockets gained this
 * from the poll()/poll_with_callback convergence: sock_ops.poll() never
 * registered a notifier before, so a blocking poll() on a socket woken purely by
 * I/O could not work (it would block forever).
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
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
  usleep(100000); // let poll() block first
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

  // Arrange the datagram to arrive only after poll() is already blocking, so it
  // can only complete by being woken - not by the initial readiness derivation.
#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t t;
  assert(pthread_create(&t, NULL, sender, NULL) == 0);
#else
  emscripten_async_call(send_ping, NULL, 100);
#endif

  struct pollfd pfd = { .fd = rx, .events = POLLIN };
  int n = poll(&pfd, 1, -1); // blocks; only the arrival can wake it
  assert(n == 1 && (pfd.revents & POLLIN));

  char buf[4];
  assert(recv(rx, buf, sizeof(buf), 0) == 4 && memcmp(buf, "ping", 4) == 0);

  close(rx);
  close(tx);
  printf("POLL SOCKET BLOCKING PASS\n");
  return 0;
}
