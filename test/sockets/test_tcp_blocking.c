/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Blocking TCP loopback ping/pong exercising the _emscripten_fd_wait primitive:
 * a *blocking* accept() and a *blocking* recv() that each have to suspend. The
 * client connects from a separate thread after a delay, so the server's accept
 * and recv both would-block first and can only complete by being woken through
 * the inode readiness wait-queue (the SOCKFS.emit bridge). Under
 * PROXY_TO_PTHREAD every blocking call parks its proxied worker on the
 * sync-proxy; the main-thread event loop drives node's sockets and delivers the
 * wakes. send()/write() never block (node buffers), so only the read side waits.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static struct sockaddr_in server_addr;

static void* client_thread(void* arg) {
  usleep(100000); // let the server block in accept() first
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  assert(connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0);
  assert(send(fd, "ping", 4, 0) == 4); // buffered, never blocks
  char buf[4];
  assert(recv(fd, buf, sizeof(buf), 0) == 4 && memcmp(buf, "pong", 4) == 0);
  close(fd);
  return NULL;
}

int main(void) {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0);
  socklen_t l = sizeof(server_addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&server_addr, &l) == 0);
  assert(listen(listen_fd, 4) == 0);

  pthread_t t;
  assert(pthread_create(&t, NULL, client_thread, NULL) == 0);

  // Blocking accept(): no connection is pending yet (the client sleeps first),
  // so it suspends the proxied worker on the listener's readiness queue until
  // the client connects.
  struct sockaddr_in ca;
  socklen_t cl = sizeof(ca);
  int peer_fd = accept(listen_fd, (struct sockaddr*)&ca, &cl);
  assert(peer_fd >= 0);

  // Blocking recv(): suspends until the client's "ping" arrives.
  char buf[4];
  assert(recv(peer_fd, buf, sizeof(buf), 0) == 4 && memcmp(buf, "ping", 4) == 0);
  assert(send(peer_fd, "pong", 4, 0) == 4);

  assert(pthread_join(t, NULL) == 0);
  close(peer_fd);
  close(listen_fd);
  printf("done\n");
  return 0;
}
