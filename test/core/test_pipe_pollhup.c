/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Once every write end of a pipe is closed, poll on the read end must report
// POLLHUP (and POLLIN, since read returns EOF), matching Linux.

#include <poll.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int poll_read(int fd) {
  struct pollfd pfd = {fd, POLLIN, 0};
  assert(poll(&pfd, 1, 0) >= 0);
  return pfd.revents;
}

int main() {
  const char *t = "test\n";
  int p[2];

  assert(pipe(p) == 0);

  // Nothing written yet, writer still open: not readable, no hangup.
  assert(poll_read(p[0]) == 0);

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
  printf("done\n");
  return 0;
}
