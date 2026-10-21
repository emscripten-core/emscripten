/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Calling listen() on a port that is already in use must surface EADDRINUSE via
 * SO_ERROR and the socket error callback, and report POLLERR in poll/select.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int listen_fd = -1;
int test_port = 0;
int phase = 1;

#ifdef __EMSCRIPTEN__
int callback_count = 0;

void error_callback(int fd, int err, const char* msg, void* userData) {
  assert(fd == listen_fd);
  printf("error_callback: fd=%d errno=%d (%s) msg=%s\n", fd, err, strerror(err), msg);
  assert(err == EADDRINUSE);
  assert(strstr(msg, "EADDRINUSE") != NULL);
  callback_count++;
}
#endif

void test_success(void) {
  printf("done\n");
  if (listen_fd >= 0) close(listen_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void start_phase2(void);

void main_loop(void) {
  struct pollfd pfd = {listen_fd, POLLIN, 0};
  int p = poll(&pfd, 1, 0);

  if (p > 0 && (pfd.revents & POLLERR)) {
    printf("phase %d: poll revents 0x%x (POLLERR)\n", phase, pfd.revents);

    if (phase == 1) {
      // Phase 1: Retrieve and consume error via getsockopt(SO_ERROR).
      int err = 0;
      socklen_t l = sizeof(err);
      assert(getsockopt(listen_fd, SOL_SOCKET, SO_ERROR, &err, &l) == 0);
      printf("SO_ERROR: %d (%s)\n", err, strerror(err));
      assert(err == EADDRINUSE);

      // Reading SO_ERROR a second time returns 0 (error was consumed).
      err = 0;
      assert(getsockopt(listen_fd, SOL_SOCKET, SO_ERROR, &err, &l) == 0);
      assert(err == 0);

      // After SO_ERROR is cleared, poll reports POLLHUP on the dead listener.
      p = poll(&pfd, 1, 0);
      assert(p > 0);
      assert(pfd.revents & POLLHUP);

      // Calling accept() on the dead listener fails with EINVAL.
      int cfd = accept(listen_fd, NULL, NULL);
      assert(cfd == -1);
      assert(errno == EINVAL);

#ifdef __EMSCRIPTEN__
      assert(callback_count == 1);
#endif
      close(listen_fd);
      listen_fd = -1;

      // Move to Phase 2 to test error consumption via accept().
      start_phase2();
    } else if (phase == 2) {
      // Phase 2: Consume error directly via accept().
      int cfd = accept(listen_fd, NULL, NULL);
      assert(cfd == -1);
      assert(errno == EADDRINUSE);

      // accept() consumed the error, so getsockopt(SO_ERROR) returns 0.
      int err = 0;
      socklen_t l = sizeof(err);
      assert(getsockopt(listen_fd, SOL_SOCKET, SO_ERROR, &err, &l) == 0);
      printf("after accept SO_ERROR: %d\n", err);
      assert(err == 0);

      // Dead listener reports POLLHUP.
      p = poll(&pfd, 1, 0);
      assert(p > 0);
      assert(pfd.revents & POLLHUP);

      // Subsequent accept() fails with EINVAL.
      cfd = accept(listen_fd, NULL, NULL);
      assert(cfd == -1);
      assert(errno == EINVAL);

#ifdef __EMSCRIPTEN__
      assert(callback_count == 2);
#endif
      test_success();
    }
  }
}

int create_listen_socket(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  assert(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(fd, 5) == 0);
  return fd;
}

void start_phase2(void) {
  phase = 2;
  listen_fd = create_listen_socket(test_port);
}

int main(int argc, char** argv) {
  assert(argc > 1);
  test_port = atoi(argv[1]);
  assert(test_port > 0);

#ifdef __EMSCRIPTEN__
  emscripten_set_socket_error_callback(NULL, error_callback);
#endif

  listen_fd = create_listen_socket(test_port);

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
