/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Under plain -pthread, main() sits in pthread_join on the main thread: its
 * futex wait runs proxied calls but never returns to the event loop. A
 * thread's blocking accept() and recv() that can complete at once (connection
 * and data already pending) must deliver their results without a microtask,
 * or the thread waits forever on a call that has already finished.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int listen_fd = -1, client_fd = -1;
struct sockaddr_in addr;

void* accept_and_recv(void* arg) {
  int peer_fd = accept(listen_fd, NULL, NULL);
  assert(peer_fd >= 0);
  char buf[8];
  assert(recv(peer_fd, buf, sizeof(buf), 0) == 4);
  assert(memcmp(buf, "ping", 4) == 0);
  close(peer_fd);
  return NULL;
}

void join_thread(void* arg) {
  pthread_t t;
  assert(pthread_create(&t, NULL, accept_and_recv, NULL) == 0);
  assert(pthread_join(t, NULL) == 0);
  close(client_fd);
  close(listen_fd);
  printf("done\n");
  emscripten_force_exit(0);
}

void send_ping(void* arg) {
  assert(send(client_fd, "ping", 4, 0) == 4);
  // Let the data land on the listener's side before main() blocks.
  emscripten_async_call(join_thread, NULL, 100);
}

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

  client_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(client_fd >= 0);
  int r = connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));
  assert(r == 0 || errno == EINPROGRESS);
  // The connection completes on the event loop before anything blocks.
  emscripten_async_call(send_ping, NULL, 100);
  emscripten_exit_with_live_runtime();
}
