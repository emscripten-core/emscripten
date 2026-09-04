/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Threads blocked indefinitely in epoll_wait() and poll() on a socket are
 * canceled with pthread_cancel. Each must exit with PTHREAD_CANCELED, leaving
 * its readiness listener registered on the main thread. A datagram then fires
 * those orphaned listeners - resolving proxied waits whose callers are gone -
 * which must be harmless, and the socket must still be usable afterwards.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

static int rx, tx, ep;
static struct sockaddr_in addr;

static void* epoll_waiter(void* arg) {
  struct epoll_event out;
  epoll_wait(ep, &out, 1, -1);
  assert(0 && "epoll_wait should have been canceled");
  return NULL;
}

static void* poll_waiter(void* arg) {
  struct pollfd pfd = {.fd = rx, .events = POLLIN};
  poll(&pfd, 1, -1);
  assert(0 && "poll should have been canceled");
  return NULL;
}

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

  ep = epoll_create1(0);
  assert(ep >= 0);
  struct epoll_event ev = {.events = EPOLLIN};
  ev.data.fd = rx;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, rx, &ev) == 0);

  pthread_t t1, t2;
  assert(pthread_create(&t1, NULL, epoll_waiter, NULL) == 0);
  assert(pthread_create(&t2, NULL, poll_waiter, NULL) == 0);
  usleep(100000); // let both park in their waits first

  void* ret;
  assert(pthread_cancel(t1) == 0);
  assert(pthread_join(t1, &ret) == 0 && ret == PTHREAD_CANCELED);
  assert(pthread_cancel(t2) == 0);
  assert(pthread_join(t2, &ret) == 0 && ret == PTHREAD_CANCELED);

  // Fire the listeners the canceled waits left behind.
  assert(sendto(tx, "ping", 4, 0, (struct sockaddr*)&addr, sizeof(addr)) == 4);
  usleep(100000); // let the late completions run on the main thread

  struct epoll_event out;
  assert(epoll_wait(ep, &out, 1, -1) == 1 && out.data.fd == rx);
  char buf[4];
  assert(recvfrom(rx, buf, sizeof(buf), 0, NULL, NULL) == 4);
  assert(memcmp(buf, "ping", 4) == 0);

  close(ep);
  close(rx);
  close(tx);
  printf("done\n");
  return 0;
}
