/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * accept4(SOCK_NONBLOCK) must yield a non-blocking accepted socket even off a
 * *blocking* listener: the flag is applied on top of the flags inherited from
 * the listener, not dropped. A poll()-driven main loop (single-threaded, zero
 * timeout) waits for the incoming connection, accept4()s it with SOCK_NONBLOCK,
 * then checks F_GETFL reports O_NONBLOCK and that a data-less recv() would-block
 * with EAGAIN rather than hanging. Plain POSIX, so it also runs natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static int listen_fd = -1;
static int client_fd = -1;
static int peer_fd = -1;

static void finish(void) {
  if (client_fd >= 0) close(client_fd);
  if (peer_fd >= 0) close(peer_fd);
  if (listen_fd >= 0) close(listen_fd);
  printf("done\n");
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
}

static void main_loop(void) {
  struct pollfd pfd = { .fd = listen_fd, .events = POLLIN };
  if (poll(&pfd, 1, 0) <= 0 || !(pfd.revents & POLLIN)) {
    return; // no connection queued yet
  }

  // The listener is blocking (never marked O_NONBLOCK), so inheritance alone
  // would give a blocking socket; SOCK_NONBLOCK must override that.
  peer_fd = accept4(listen_fd, NULL, NULL, SOCK_NONBLOCK);
  assert(peer_fd >= 0);

  int fl = fcntl(peer_fd, F_GETFL);
  assert(fl >= 0 && (fl & O_NONBLOCK) && "accept4 SOCK_NONBLOCK not honored");

  // A non-blocking recv with no data pending returns EAGAIN immediately instead
  // of blocking, confirming the fd is really non-blocking.
  char buf[4];
  ssize_t n = recv(peer_fd, buf, sizeof(buf), 0);
  assert(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));

  finish();
}

int main(void) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  socklen_t l = sizeof(addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &l) == 0);
  assert(listen(listen_fd, 4) == 0);
  // Deliberately leave listen_fd blocking to prove SOCK_NONBLOCK is applied on
  // top of the inherited (blocking) listener flags.

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  fcntl(client_fd, F_SETFL, O_NONBLOCK);
  int r = connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));
  assert(r == 0 || errno == EINPROGRESS);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (peer_fd < 0) {
    main_loop();
    usleep(1000);
  }
#endif
  return 0;
}
