/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Read-side flow control. The server (argv[1]) sends argv[2] bytes, but we only
 * start reading after a while, and tell the server so by sending one byte. The
 * server checks that it couldn't send everything meanwhile, i.e. that received
 * data doesn't pile up without limit while nobody reads. We check that all data
 * then arrives intact, up to the server's EOF. This sleeps and blocks in
 * select(), so it needs a thread that can wait (e.g. -sPROXY_TO_PTHREAD).
 * Plain POSIX, also runs natively.
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
  assert(argc > 2 && "usage: test_tcp_recv_flow_control <port> <bytes>");
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

  // Let the server send as much as it can while we don't read.
  sleep(2);
  assert(send(fd, "r", 1, 0) == 1);

  // Byte i of the stream is i % 251.
  static unsigned char buf[65536];
  long long received = 0;
  while (true) {
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n < 0) {
      assert((errno == EAGAIN || errno == EWOULDBLOCK) && "recv failed");
      wait_for(fd, false);
      continue;
    }
    if (n == 0) {
      break;
    }
    for (ssize_t i = 0; i < n; i++) {
      assert(buf[i] == (unsigned char)((received + i) % 251) && "corrupt data");
    }
    received += n;
  }
  assert(received == total && "not all data arrived");
  printf("received %lld bytes\n", received);
  close(fd);
  printf("done\n");
  return 0;
}
