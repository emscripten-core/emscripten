/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * close() must deliver the data sent before it, also when the peer has already
 * finished its side. The server (argv[1]) half-closes right away and only
 * starts reading after a while. We wait for its EOF, send argv[2] bytes over a
 * non-blocking socket, and close() as soon as send() has accepted the last of
 * them, while much of the data is still buffered. The server checks that all
 * of it arrives. Plain POSIX, also runs natively.
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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int fd = -1;
bool connected = false;
bool peer_finished = false;
char chunk[65536];
long long sent = 0;
long long total = 0;

void test_success(void) {
  printf("done\n");
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(fd, &fdr);
  FD_SET(fd, &fdw);
  select(fd + 1, &fdr, &fdw, NULL, &tv);

  if (!connected && FD_ISSET(fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &l);
    assert(err == 0 && "connect failed");
    connected = true;
  }

  if (connected && !peer_finished && FD_ISSET(fd, &fdr)) {
    char c;
    ssize_t n = recv(fd, &c, 1, 0);
    if (n < 0) {
      assert((errno == EAGAIN || errno == EWOULDBLOCK) && "recv failed");
      return;
    }
    assert(n == 0 && "expected the server to finish its side");
    peer_finished = true;
  }

  if (!peer_finished) return;

  while (sent < total) {
    size_t len = total - sent < (long long)sizeof(chunk) ? (size_t)(total - sent) : sizeof(chunk);
    ssize_t n = send(fd, chunk, len, 0);
    if (n < 0) {
      assert((errno == EAGAIN || errno == EWOULDBLOCK) && "send failed");
      return; // try again on the next iteration
    }
    sent += n;
  }

  // Everything has been accepted, but not yet delivered.
  close(fd);
  test_success();
}

int main(int argc, char** argv) {
  assert(argc > 2 && "usage: test_tcp_close_flush <port> <bytes>");
  total = atoll(argv[2]);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(atoi(argv[1]));
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

  int r = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
  assert((r == 0 || errno == EINPROGRESS) && "connect");

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (1) {
    main_loop();
    usleep(1000);
  }
#endif
  return 0;
}
