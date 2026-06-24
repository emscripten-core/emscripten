/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Verifies emscripten_poll_with_callback against a real socket: a datagram
 * arriving on a bound UDP socket wakes the callback through the SOCKFS readiness
 * -> poll-callback bridge, with no select and no main loop.
 * Also checks the capability gate: a descriptor type that cannot deliver
 * readiness callbacks (a regular file) is rejected with -EPERM even though it is
 * always "ready", and a bad fd with -EBADF.
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

static int rx = -1, tx = -1;
static volatile int cancel_revents = -1;

static void fail(const char* why) {
  printf("POLL CALLBACK FAIL: %s\n", why);
  abort();
}

static void on_cancel(int fd, int revents) {
  cancel_revents = revents;
}

// revents arrives by value - nothing to own or free.
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

  // A listening socket can't yet deliver accept readiness through the seam, so
  // it is reported unpollable rather than arming a waiter that never fires.
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(lfd >= 0);
  struct sockaddr_in laddr;
  memset(&laddr, 0, sizeof(laddr));
  laddr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &laddr.sin_addr);
  assert(bind(lfd, (struct sockaddr*)&laddr, sizeof(laddr)) == 0);
  assert(listen(lfd, 1) == 0);
  if (emscripten_poll_with_callback(lfd, POLLIN, 0, on_readable) != -EPERM) {
    fail("listening socket should be rejected with -EPERM");
  }
  close(lfd);

  // Closing an fd with a pending waiter delivers POLLNVAL (no leak, no hang).
  // The callback fires on a later tick, so it's checked in on_readable below.
  int cfd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(cfd >= 0);
  if (emscripten_poll_with_callback(cfd, POLLIN, -1, on_cancel) != 0) {
    fail("poll_with_callback did not arm for cancel");
  }
  close(cfd);

  rx = socket(AF_INET, SOCK_DGRAM, 0);
  tx = socket(AF_INET, SOCK_DGRAM, 0);
  assert(rx >= 0 && tx >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0); // ephemeral
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  if (bind(rx, (struct sockaddr*)&addr, sizeof(addr)) != 0) fail("bind");

  socklen_t l = sizeof(addr);
  if (getsockname(rx, (struct sockaddr*)&addr, &l) != 0) fail("getsockname");

  // Arm the callback before the datagram is sent: rx isn't readable yet, so this
  // registers a waiter that the arriving datagram (SOCKFS 'message' emit) wakes.
  if (emscripten_poll_with_callback(rx, POLLIN, -1, on_readable) != 0) {
    fail("poll_with_callback did not arm");
  }

  if (sendto(tx, "ping", 4, 0, (struct sockaddr*)&addr, sizeof(addr)) != 4) {
    fail("sendto");
  }

  return 0;
}
