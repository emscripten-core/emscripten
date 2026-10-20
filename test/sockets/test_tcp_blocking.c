/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Blocking accept() and recv() on real sockets. Each blocking call is made
 * with nothing to consume and is woken by an event that arrives *after* it has
 * blocked - the peer acts on a delay (from another thread under -pthread, or a
 * timer under JSPI) - so the call must suspend (the proxied worker under
 * PROXY_TO_PTHREAD, or the calling stack under JSPI) and be woken through the
 * socket's readiness wait-queue. Non-blocking calls and MSG_DONTWAIT on the
 * same empty sockets still return EAGAIN immediately, and a call with data
 * already pending returns without waiting.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif

int listen_fd = -1, client_fd = -1, peer_fd = -1;
struct sockaddr_in addr;

void start_connect(void* arg) {
  client_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(client_fd >= 0);
  int r = connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));
  assert(r == 0 || errno == EINPROGRESS);
}

void send_ping(void* arg) {
  assert(send(client_fd, "ping", 4, 0) == 4);
}

void close_client(void* arg) {
  close(client_fd);
  client_fd = -1;
}

// Run `fn` only once the blocking call on the main stack has parked.
#ifdef __EMSCRIPTEN_PTHREADS__
// Under PROXY_TO_PTHREAD main() runs on a worker parked in the blocking call,
// so its event loop can't fire a timer - the wake is a cross-thread notify
// from a second thread.
void* delayed(void* fn) {
  usleep(100000);
  ((void (*)(void*))fn)(NULL);
  return NULL;
}

void later(void (*fn)(void*)) {
  pthread_t t;
  assert(pthread_create(&t, NULL, delayed, (void*)fn) == 0);
  assert(pthread_detach(t) == 0);
}
#else
void later(void (*fn)(void*)) {
  emscripten_async_call(fn, NULL, 100);
}
#endif

int main(void) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(listen_fd, 4) == 0);
  socklen_t l = sizeof(addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &l) == 0);

  // Non-blocking: nothing pending is EAGAIN right away, never a wait.
  assert(fcntl(listen_fd, F_SETFL, O_NONBLOCK) == 0);
  assert(accept(listen_fd, NULL, NULL) == -1 && errno == EAGAIN);
  assert(fcntl(listen_fd, F_SETFL, 0) == 0);

  // Blocking accept(), woken by a connection that arrives after it blocked.
  later(start_connect);
  struct sockaddr_in peer;
  socklen_t pl = sizeof(peer);
  peer_fd = accept(listen_fd, (struct sockaddr*)&peer, &pl);
  assert(peer_fd >= 0);
  assert(peer.sin_family == AF_INET && ntohs(peer.sin_port) != 0);
  assert(!(fcntl(peer_fd, F_GETFL) & O_NONBLOCK));

  // Blocking recv(): MSG_DONTWAIT on the empty socket is EAGAIN at once, then
  // a plain recv() blocks until the peer's data arrives.
  char buf[8];
  assert(recv(peer_fd, buf, sizeof(buf), MSG_DONTWAIT) == -1 && errno == EAGAIN);
  later(send_ping);
  assert(recv(peer_fd, buf, sizeof(buf), 0) == 4);
  assert(memcmp(buf, "ping", 4) == 0);

  // Blocking recv() woken by the peer closing: EOF.
  later(close_client);
  assert(recv(peer_fd, buf, sizeof(buf), 0) == 0);
  close(peer_fd);

  // A connection already pending when accept() is called (poll() has reported
  // the listener readable) is returned without waiting, and data already
  // queued satisfies recv() without waiting.
  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  struct pollfd p = { .fd = listen_fd, .events = POLLIN };
  assert(poll(&p, 1, -1) == 1 && (p.revents & POLLIN));
  peer_fd = accept(listen_fd, NULL, NULL);
  assert(peer_fd >= 0);
  assert(send(client_fd, "pong", 4, 0) == 4);
  p.fd = peer_fd;
  assert(poll(&p, 1, -1) == 1 && (p.revents & POLLIN));
  assert(recv(peer_fd, buf, sizeof(buf), 0) == 4);
  assert(memcmp(buf, "pong", 4) == 0);
  close(peer_fd);
  close(client_fd);

  close(listen_fd);
  printf("done\n");
  return 0;
}
