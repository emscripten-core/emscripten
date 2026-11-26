/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Write-side backpressure, with resuming. We send argv[2] bytes over a
 * non-blocking socket to a server (argv[1]) that only starts reading after a
 * while. send() must report EAGAIN at some point rather than buffering
 * everything, and select() must report the socket writable again once the
 * server reads, so that all data arrives. The server checks the data and then
 * closes the connection. select() has to block, so this needs a thread that
 * can wait (e.g. -sPROXY_TO_PTHREAD). Plain POSIX, also runs natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static void wait_for(int fd, bool write) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval tv = {.tv_sec = 30};
  int n = select(fd + 1, write ? NULL : &fds, write ? &fds : NULL, NULL, &tv);
  assert(n == 1 && FD_ISSET(fd, &fds) && "timed out waiting for the socket");
}

int main(int argc, char** argv) {
  assert(argc > 2 && "usage: test_tcp_backpressure_resume <port> <bytes>");
  long long total = atoll(argv[2]);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(atoi(argv[1]));
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

  int r = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
  assert((r == 0 || errno == EINPROGRESS) && "connect");
  wait_for(fd, true);
  int err = 0;
  socklen_t len = sizeof(err);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
  assert(err == 0 && "connect failed");

  // Byte i of the stream is i % 251, which the server checks.
  static unsigned char chunk[65536];
  long long sent = 0;
  bool would_block = false;
  while (sent < total) {
    size_t n = total - sent < (long long)sizeof(chunk) ? (size_t)(total - sent) : sizeof(chunk);
    for (size_t i = 0; i < n; i++) {
      chunk[i] = (unsigned char)((sent + i) % 251);
    }
    ssize_t written = send(fd, chunk, n, 0);
    if (written < 0) {
      assert((errno == EAGAIN || errno == EWOULDBLOCK) && "send failed");
      would_block = true;
      wait_for(fd, true);
      continue;
    }
    sent += written;
  }
  assert(would_block && "send() never reported EAGAIN");
  printf("sent %lld bytes\n", sent);

  // The server closes the connection once it has checked all data.
  char c;
  wait_for(fd, false);
  assert(recv(fd, &c, 1, 0) == 0 && "expected the server to close");
  close(fd);
  printf("done\n");
  return 0;
}
