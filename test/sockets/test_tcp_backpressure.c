/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Write-side backpressure. We connect to a sink server (argv[1]) that accepts
 * but never reads, then send non-blocking until the kernel + node buffers fill
 * and send() reports EAGAIN. That proves writes are bounded rather than
 * buffered without limit. Plain POSIX, also runs natively.
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
static char chunk[65536];
// Safety cap so a misbehaving stack that never backpressures can't run forever.
static long long sent_total = 0;
static const long long CAP = 512LL * 1024 * 1024;

static void finish(int result) {
  printf(result == 0 ? "BACKPRESSURE PASS\n" : "BACKPRESSURE FAIL\n");
  if (fd >= 0) close(fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
  // The socket is closed and the main loop cancelled, so node's event loop
  // drains and the process exits naturally with status 0. On failure abort()
  // to surface a non-zero exit code to the test harness.
  if (result != 0) abort();
#else
  exit(result);
#endif
}

static void main_loop(void) {
  fd_set fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdw);
  FD_SET(fd, &fdw);
  select(64, NULL, &fdw, NULL, &tv);

  if (!connected && FD_ISSET(fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &l);
    if (err != 0) {
      printf("connect failed: %s\n", strerror(err));
      finish(1);
      return;
    }
    connected = true;
  }

  if (!connected) return;

  // Push hard. The peer never reads, so this must eventually would-block.
  while (sent_total < CAP) {
    ssize_t n = send(fd, chunk, sizeof(chunk), 0);
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        printf("backpressure after %lld bytes\n", sent_total);
        finish(0);
      } else {
        printf("send failed: %s\n", strerror(errno));
        finish(1);
      }
      return;
    }
    sent_total += n;
  }
  printf("no backpressure after %lld bytes\n", sent_total);
  finish(1);
}

int main(int argc, char** argv) {
  assert(argc > 1 && "usage: test_tcp_backpressure <port>");

  fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(atoi(argv[1]));
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

  int r = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
  if (r != 0 && errno != EINPROGRESS) {
    perror("connect");
    return 1;
  }

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
