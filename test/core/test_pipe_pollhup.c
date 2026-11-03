/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Once every write end of a pipe is closed, poll on the read end must report
// POLLHUP (and POLLIN, since read returns EOF), matching Linux. Symmetrically,
// once every read end is closed, poll on the write end must report POLLERR
// (while staying writable, the write itself failing with EPIPE).

#include <poll.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int poll_events(int fd, short events) {
  struct pollfd pfd = {fd, events, 0};
  assert(poll(&pfd, 1, 0) >= 0);
  return pfd.revents;
}

int poll_read(int fd) {
  return poll_events(fd, POLLIN);
}

int poll_write(int fd) {
  return poll_events(fd, POLLOUT);
}

int main() {
  const char *t = "test\n";
  int p[2];

  assert(pipe(p) == 0);

  // Nothing written yet, writer still open: not readable, no hangup.
  assert(poll_read(p[0]) == 0);

  // Both ends open: write end is writable, no error.
  int wevents = poll_write(p[1]);
  assert(wevents & POLLOUT);
  assert(!(wevents & POLLERR));

  write(p[1], t, strlen(t));

  // Data pending, writer still open: readable, no hangup.
  int revents = poll_read(p[0]);
  assert(revents & POLLIN);
  assert(!(revents & POLLHUP));

  // Close the write end. Data is still buffered, so POLLIN and POLLHUP.
  close(p[1]);
  revents = poll_read(p[0]);
  assert(revents & POLLIN);
  assert(revents & POLLHUP);

  // Drain the buffer. Read end at EOF with writer gone: POLLIN and POLLHUP.
  char buf[16];
  assert(read(p[0], buf, sizeof(buf)) == (ssize_t)strlen(t));
  revents = poll_read(p[0]);
  assert(revents & POLLIN);
  assert(revents & POLLHUP);

  close(p[0]);

  // Mirror case: close the read end and poll the write end for POLLERR.
  assert(pipe(p) == 0);
  close(p[0]);
  wevents = poll_write(p[1]);
  // Still writable per Linux, but now signalling an error.
  assert(wevents & POLLOUT);
  assert(wevents & POLLERR);
  close(p[1]);

  printf("done\n");
  return 0;
}
