/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static int server_fd;
static int client_fd;
static struct sockaddr_in destination;
static bool sent;

static void succeed(void) {
  close(server_fd);
  close(client_fd);
  puts("done");
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  _exit(0);
#endif
}

static void main_loop(void) {
  if (!sent) {
    assert(sendto(client_fd, "onetwo", 6, 0, (struct sockaddr*)&destination,
                  sizeof(destination)) == 6);
    sent = true;
  }

  // Receive the single datagram scattered across two iovecs.
  char first[3];
  char second[3];
  char control[16];
  struct sockaddr_in source;
  struct iovec iovecs[2] = {
      {.iov_base = first, .iov_len = sizeof(first)},
      {.iov_base = second, .iov_len = sizeof(second)},
  };
  struct msghdr message = {
      .msg_name = &source,
      .msg_namelen = sizeof(source),
      .msg_iov = iovecs,
      .msg_iovlen = 2,
      .msg_control = control,
      .msg_controllen = sizeof(control),
  };

  ssize_t len = recvmsg(server_fd, &message, 0);
  if (len < 0) {
    assert(errno == EAGAIN || errno == EWOULDBLOCK);
    return;
  }

  assert(len == 6);
  assert(memcmp(first, "one", 3) == 0);
  assert(memcmp(second, "two", 3) == 0);
  assert(message.msg_namelen == sizeof(struct sockaddr_in));
  assert(source.sin_family == AF_INET);
  assert(message.msg_controllen == 0);
  assert(message.msg_flags == 0);
  succeed();
}

int main(void) {
  server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  client_fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(server_fd >= 0 && client_fd >= 0);

  struct sockaddr_in address = {
      .sin_family = AF_INET,
      .sin_port = htons(0),
  };
  inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
  assert(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == 0);

  socklen_t length = sizeof(destination);
  assert(getsockname(server_fd, (struct sockaddr*)&destination, &length) == 0);
  assert(fcntl(server_fd, F_SETFL, O_NONBLOCK) == 0);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 0);
#else
  while (true) {
    main_loop();
    usleep(1000);
  }
#endif
  return 0;
}
