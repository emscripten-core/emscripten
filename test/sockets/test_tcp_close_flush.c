/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Data accepted by send() is delivered even when the sender closes (or shuts
 * down) at once. The client fills its send buffers against a peer that is not
 * yet reading, then closes; with -DSHUTDOWN it shuts down the write side
 * instead and closes once POLLOUT reports the flush and FIN completed, while
 * the server reads. The server reads until EOF and must see every byte.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int listen_fd, client_fd = -1, peer_fd = -1;
size_t sent, received;
bool closed;
char buf[65536];

bool ready(int fd, short events) {
  struct pollfd p = {fd, events, 0};
  return poll(&p, 1, 0) == 1 && (p.revents & events);
}

void finish(void) {
  printf("sent %zu received %zu\n", sent, received);
  assert(received == sent);
  printf("done\n");
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
  close(peer_fd);
  close(listen_fd);
  exit(0);
}

void main_loop(void) {
  if (peer_fd < 0) {
    peer_fd = accept(listen_fd, NULL, NULL);
    if (peer_fd < 0) return;
    fcntl(peer_fd, F_SETFL, O_NONBLOCK);
  }
  if (!sent) {
    if (!ready(client_fd, POLLOUT)) return;
    // Fill the buffers: the peer is not reading yet.
    ssize_t n;
    while ((n = send(client_fd, buf, sizeof(buf), 0)) > 0) sent += n;
    assert(errno == EAGAIN);
#ifdef SHUTDOWN
    assert(shutdown(client_fd, SHUT_WR) == 0);
#else
    close(client_fd);
    closed = true;
#endif
  }
#ifdef SHUTDOWN
  // POLLOUT after the shutdown: everything has been handed to the kernel, so
  // even an immediate close loses nothing.
  if (!closed && ready(client_fd, POLLOUT)) {
    close(client_fd);
    closed = true;
  }
#else
  if (!closed) return;
#endif
  ssize_t n;
  while ((n = recv(peer_fd, buf, sizeof(buf), 0)) > 0) received += n;
  if (n == 0) finish();
  assert(errno == EAGAIN);
}

int main(void) {
  struct sockaddr_in addr = {0};
  socklen_t len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(listen_fd, 1) == 0);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &len) == 0);
  fcntl(listen_fd, F_SETFL, O_NONBLOCK);

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(client_fd, F_SETFL, O_NONBLOCK);
  int r = connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));
  assert(r == 0 || errno == EINPROGRESS);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (1) main_loop();
#endif
  return 0;
}
