/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Verifies emscripten_poll_with_callback against real sockets, all through the
 * SOCKFS readiness -> poll-callback bridge, with no select and no main loop:
 *   - a datagram arriving on a bound UDP socket wakes the callback (POLLIN);
 *   - a listening socket is readable when a client is queued for accept;
 *   - closing an fd with a pending waiter delivers POLLNVAL;
 *   - the capability gate rejects a regular file (-EPERM, even though it is
 *     always "ready") and a bad fd (-EBADF).
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int rx = -1, tx = -1, lfd = -1, client = -1;
static volatile int cancel_revents = -1;
static struct sockaddr_in rx_addr;

static void fail(const char* why) {
  printf("POLL CALLBACK FAIL: %s\n", why);
  abort();
}

static void on_cancel(int fd, int revents) {
  cancel_revents = revents;
}

static void on_readable(int fd, int revents) {
  if (!(revents & POLLIN)) fail("socket reported not readable");
  char buf[4];
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  if (n != 4 || memcmp(buf, "ping", 4) != 0) fail("did not receive datagram");
  // The earlier close()-cancellation must have delivered POLLNVAL by now.
  if (cancel_revents != POLLNVAL) fail("close did not deliver POLLNVAL");
  close(rx);
  close(tx);
  printf("POLL CALLBACK PASS\n");
}

// Listener is readable once a client is queued; accept, then chain the
// datagram phase for deterministic ordering.
static void on_accept(int fd, int revents) {
  if (!(revents & POLLIN)) fail("listener reported not readable");
  struct sockaddr_in caddr;
  socklen_t clen = sizeof(caddr);
  int c = accept(lfd, (struct sockaddr*)&caddr, &clen);
  if (c < 0) fail("accept after readiness");
  close(c);
  close(lfd);
  close(client);

  if (emscripten_poll_with_callback(rx, POLLIN, -1, on_readable) != 0) {
    fail("poll_with_callback did not arm");
  }
  if (sendto(tx, "ping", 4, 0, (struct sockaddr*)&rx_addr, sizeof(rx_addr)) != 4) {
    fail("sendto");
  }
}

int main(void) {
  // Capability gate: a regular file can't deliver readiness callbacks, so it is
  // rejected with -EPERM even though a regular file is always ready.
  int filefd = open("/tmp/poll_cb.tmp", O_CREAT | O_RDWR, 0600);
  assert(filefd >= 0);
  if (emscripten_poll_with_callback(filefd, POLLIN, 0, on_readable) != -EPERM) {
    fail("regular file should be rejected with -EPERM");
  }
  close(filefd);

  // A bad fd is -EBADF.
  if (emscripten_poll_with_callback(9999, POLLIN, 0, on_readable) != -EBADF) {
    fail("invalid fd should be rejected with -EBADF");
  }

  // Closing an fd with a pending waiter delivers POLLNVAL (checked in on_readable).
  int cfd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(cfd >= 0);
  if (emscripten_poll_with_callback(cfd, POLLIN, -1, on_cancel) != 0) {
    fail("poll_with_callback did not arm for cancel");
  }
  close(cfd);

  // Prepare the datagram receiver up front; it is armed and fed from on_accept.
  rx = socket(AF_INET, SOCK_DGRAM, 0);
  tx = socket(AF_INET, SOCK_DGRAM, 0);
  assert(rx >= 0 && tx >= 0);
  memset(&rx_addr, 0, sizeof(rx_addr));
  rx_addr.sin_family = AF_INET;
  rx_addr.sin_port = htons(0); // ephemeral
  inet_pton(AF_INET, "127.0.0.1", &rx_addr.sin_addr);
  if (bind(rx, (struct sockaddr*)&rx_addr, sizeof(rx_addr)) != 0) fail("bind rx");
  socklen_t l = sizeof(rx_addr);
  if (getsockname(rx, (struct sockaddr*)&rx_addr, &l) != 0) fail("getsockname rx");

  // A listening socket is pollable: arm a callback, then connect a client. The
  // queued connection wakes the listener with POLLIN (see on_accept).
  lfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(lfd >= 0);
  struct sockaddr_in laddr;
  memset(&laddr, 0, sizeof(laddr));
  laddr.sin_family = AF_INET;
  laddr.sin_port = htons(0);
  inet_pton(AF_INET, "127.0.0.1", &laddr.sin_addr);
  assert(bind(lfd, (struct sockaddr*)&laddr, sizeof(laddr)) == 0);
  socklen_t ll = sizeof(laddr);
  assert(getsockname(lfd, (struct sockaddr*)&laddr, &ll) == 0);
  assert(listen(lfd, 1) == 0);
  if (emscripten_poll_with_callback(lfd, POLLIN, -1, on_accept) != 0) {
    fail("listening socket should arm");
  }
  client = socket(AF_INET, SOCK_STREAM, 0);
  assert(client >= 0);
  // connect is async; EINPROGRESS is expected, not an error.
  connect(client, (struct sockaddr*)&laddr, sizeof(laddr));

  return 0;
}
