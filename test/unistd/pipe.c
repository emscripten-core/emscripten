/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

unsigned char buf[1 << 16];

#define FALSE 0
#define TRUE 1

// This test program relies on the simplest read/write behavior when
// all the data can be read/written in one call.

void test_write(int fd1, unsigned char *ch, int size) {
  memset(buf, 0, sizeof buf);
  for (int i = 0; i < size; ++i) {
    buf[i] = (*ch)++;
  }
  assert(write(fd1, buf, size) == size);
}

void test_read(int fd0, unsigned char *ch, int size) {
  memset(buf, 0, sizeof buf);
  assert(read(fd0, buf, size) == size);
  for (int i = 0; i < sizeof buf; ++i) {
    unsigned char correct_ch = (i < size) ? (*ch)++ : 0;
    assert(buf[i] == correct_ch);
  }
}

void test_poll(int *fd, int data_available) {
  struct pollfd pfds[2];
  memset(pfds, 0, sizeof pfds);
  pfds[0].fd = fd[0];
  pfds[0].events = POLLIN | POLLOUT;
  pfds[1].fd = fd[1];
  pfds[1].events = POLLIN | POLLOUT;

  int ret = poll(pfds, 2, 0);
  if (data_available) {
    assert(ret == 2);
    assert(pfds[0].revents == POLLIN);
  } else {
    assert(ret == 1);
    assert(pfds[0].revents == 0);
  }
  assert(pfds[1].revents == POLLOUT);
}

int main() {
  int fd[2];
  unsigned char wchar = 0;
  unsigned char rchar = 0;

  assert(pipe(fd) == 0);

  // Test that pipe is not seekable

  memset(buf, 0, sizeof buf);
  assert(write(fd[1], buf, 128) == 128);
  assert(lseek(fd[0], 0, SEEK_CUR) == -1);
  assert(errno == ESPIPE);
  assert(lseek(fd[1], 0, SEEK_CUR) == -1);
  assert(errno == ESPIPE);
  assert(read(fd[0], buf, sizeof buf) == 128);

  // Now pipe is empty
  // Test interleaved writing and reading of different buffer sizes

  // write about 40 Kb of data
  for (int i = 1; i < 200; ++i) {
    test_write(fd[1], &wchar, i + 2);
    test_poll(fd, TRUE);
    test_read (fd[0], &rchar, i);
    test_poll(fd, TRUE);
    test_write(fd[1], &wchar, i + 1);
    test_poll(fd, TRUE);
    test_read (fd[0], &rchar, i + 3);
    test_poll(fd, FALSE);
  }

  // Test reading when there is less data available than the read buffer size

  assert(write(fd[1], buf, 10) == 10);
  assert(read(fd[0], buf, sizeof buf) == 10);

  // Write total of 1 Mb of data in small chunks
  // The pipe should not overflow

  int bytes_to_write = 1 << 20;
  while (bytes_to_write > 0) {
    test_write(fd[1], &wchar, sizeof buf);
    test_read (fd[0], &rchar, sizeof buf);
    bytes_to_write -= sizeof buf;
  }

  // Write large chunks of data (supposed to be larger than one internal buffer)
  test_write(fd[1], &wchar, 123);
  test_write(fd[1], &wchar, (1 << 15) + 321);
  test_write(fd[1], &wchar, 456);
  test_read(fd[0], &rchar, 456);
  test_read(fd[0], &rchar, (1 << 15) + 123);
  test_read(fd[0], &rchar, 321);

#ifndef WASMFS // TODO: fcntl in WASMFS
  // Test non-blocking read from empty pipe
  assert(fcntl(fd[0], F_SETFL, O_NONBLOCK) == 0);
  assert(read(fd[0], buf, sizeof buf) == -1);
  assert(errno == EAGAIN);
#endif

  // Normal operations still work in non-blocking mode
  test_poll(fd, FALSE);
  test_write(fd[1], &wchar, 10);
  test_poll(fd, TRUE);
  test_read (fd[0], &rchar, 10);
  test_poll(fd, FALSE);

  // Clear buffer
  memset(buf, 0, sizeof(buf));
  // Test closing pipes.
  // Write to pipe
  assert(write(fd[1], "XXXX", 4) == 4);
  // Close write end
  assert(close(fd[1]) == 0);
  // This write should fail
  assert(write(fd[1], "YYYY", 4) == -1);
  // The error number is EBADF
  assert(errno == EBADF);

  // read from the other end of the pipe
  assert(read(fd[0], buf, 5) == 4);
  // We should have read what we wrote to the other end
  assert(memcmp(buf, "XXXX", 4) == 0);

  // Close the read end
  assert(close(fd[0]) == 0);
  // Now reading should return an error
  assert(read(fd[0], buf, 5) == -1);
  // The error number is EBADF
  assert(errno == EBADF);

  puts("success");
  return 0;
}
