/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#define CAPACITY 65536

int writable(int fd) {
  struct pollfd pfd = {fd, POLLOUT, 0};
  assert(poll(&pfd, 1, 0) >= 0);
  return pfd.revents & POLLOUT;
}

size_t drain(int fd) {
  char buf[8192] = {0};
  size_t total = 0;
  ssize_t n;
  while ((n = read(fd, buf, sizeof buf)) > 0)
    total += n;
  assert(n == -1 && errno == EAGAIN);
  return total;
}

// Bytes go through the pipe as a running counter so wrap-around and growth
// bugs show up as data out of order.
unsigned char next_out, next_in;

void write_seq(int fd, size_t n) {
  static unsigned char data[CAPACITY];
  for (size_t i = 0; i < n; i++)
    data[i] = next_out++;
  assert(write(fd, data, n) == n);
}

void read_seq(int fd, size_t n) {
  // The guard catches a read that writes before the start of `data`.
  static struct {
    unsigned char guard[CAPACITY];
    unsigned char data[CAPACITY];
  } dest;
  assert(read(fd, dest.data, n) == n);
  for (size_t i = 0; i < n; i++)
    assert(dest.data[i] == next_in++);
  for (size_t i = 0; i < CAPACITY; i++)
    assert(dest.guard[i] == 0);
}

int main() {
  int p[2];
  assert(pipe(p) == 0);
  assert(fcntl(p[1], F_SETFL, O_NONBLOCK) == 0);
  assert(writable(p[1]));

  char chunk[PIPE_BUF] = {0};
  char buf[8192] = {0};

  size_t total = 0;
  ssize_t n;
  while ((n = write(p[1], chunk, sizeof chunk)) > 0)
    total += n;
  assert(n == -1 && errno == EAGAIN);
  assert(total == CAPACITY);
  assert(!writable(p[1]));

  // PIPE_BUF rule with 100 bytes of room.
  assert(read(p[0], buf, 100) == 100);
  assert(!writable(p[1]));
  assert(write(p[1], chunk, 200) == -1 && errno == EAGAIN);
  assert(write(p[1], chunk, 5000) == 100);
  assert(write(p[1], chunk, 1) == -1 && errno == EAGAIN);

  assert(read(p[0], buf, PIPE_BUF) == PIPE_BUF);
  assert(writable(p[1]));
  assert(write(p[1], chunk, PIPE_BUF) == PIPE_BUF);
  assert(!writable(p[1]));

  assert(drain(p[0]) == CAPACITY);
  assert(writable(p[1]));

  assert(write(p[1], "ab", 2) == 2);
  assert(close(p[1]) == 0);
  assert(read(p[0], buf, sizeof buf) == 2);
  assert(read(p[0], buf, sizeof buf) == 0);
  assert(close(p[0]) == 0);

  assert(pipe(p) == 0);
  assert(close(p[0]) == 0);
  assert(write(p[1], "x", 1) == -1 && errno == EPIPE);
  assert(close(p[1]) == 0);

  // Wrap around the end of the initial buffer, grow while wrapped, then wrap
  // the full-size buffer.
  assert(pipe(p) == 0);
  assert(fcntl(p[1], F_SETFL, O_NONBLOCK) == 0);
  write_seq(p[1], 6000);
  read_seq(p[0], 5000);
  write_seq(p[1], 4000);
  write_seq(p[1], 8000);
  read_seq(p[0], 2000);
  write_seq(p[1], 50000);
  read_seq(p[0], 60000);
  write_seq(p[1], 40000);
  read_seq(p[0], 6000);
  read_seq(p[0], 35000);
  assert(drain(p[0]) == 0);
  write_seq(p[1], CAPACITY);
  assert(!writable(p[1]));
  read_seq(p[0], CAPACITY);
  assert(close(p[0]) == 0);
  assert(close(p[1]) == 0);

  printf("done\n");
  return 0;
}
