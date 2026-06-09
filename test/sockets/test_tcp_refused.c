/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A non-blocking connect to a loopback port with nothing listening must
 * surface ECONNREFUSED via SO_ERROR. Self-contained and also runs natively.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
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

static void finish(int result) {
  printf(result == 0 ? "REFUSED PASS\n" : "REFUSED FAIL\n");
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

  if (FD_ISSET(fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &l);
    if (err == 0) return; // not resolved yet
    printf("connect resolved with errno %d (%s)\n", err, strerror(err));
    finish(err == ECONNREFUSED ? 0 : 1);
  }
}

int main(void) {
  fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(1); // nothing listens on loopback port 1
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

  // A non-blocking connect may return 0 (emscripten) or -1/EINPROGRESS
  // (native), or refuse synchronously. The async failure is checked via
  // SO_ERROR in the main loop below.
  int r = connect(fd, (struct sockaddr*)&dest, sizeof(dest));
  if (r == -1 && errno == ECONNREFUSED) {
    printf("connect resolved with errno %d (%s)\n", errno, strerror(errno));
    printf("REFUSED PASS\n");
    return 0;
  }
  if (r == -1 && errno != EINPROGRESS) {
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
