/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static int server_fd;
static int client_fd;
static struct sockaddr_in destination;
static bool sent;
static unsigned int received;

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
    struct iovec siov[2] = {
        {.iov_base = "one", .iov_len = 3},
        {.iov_base = "two", .iov_len = 3},
    };
    struct mmsghdr smsgs[2];
    memset(smsgs, 0, sizeof(smsgs));
    for (int i = 0; i < 2; i++) {
      smsgs[i].msg_hdr.msg_name = &destination;
      smsgs[i].msg_hdr.msg_namelen = sizeof(destination);
      smsgs[i].msg_hdr.msg_iov = &siov[i];
      smsgs[i].msg_hdr.msg_iovlen = 1;
    }
    assert(sendmmsg(client_fd, smsgs, 2, 0) == 2);
    assert(smsgs[0].msg_len == 3 && smsgs[1].msg_len == 3);
    sent = true;
  }

  char buffers[2][3];
  char controls[2][16];
  struct sockaddr_in sources[2];
  struct iovec iovecs[2];
  struct mmsghdr messages[2];
  memset(messages, 0, sizeof(messages));
  for (int i = 0; i < 2; i++) {
    iovecs[i] = (struct iovec){.iov_base = buffers[i], .iov_len = sizeof(buffers[i])};
    messages[i].msg_hdr.msg_name = &sources[i];
    messages[i].msg_hdr.msg_namelen = sizeof(sources[i]);
    messages[i].msg_hdr.msg_iov = &iovecs[i];
    messages[i].msg_hdr.msg_iovlen = 1;
    messages[i].msg_hdr.msg_control = controls[i];
    messages[i].msg_hdr.msg_controllen = sizeof(controls[i]);
  }

  int count = recvmmsg(server_fd, messages, 2, MSG_DONTWAIT, NULL);
  if (count < 0) {
    assert(errno == EAGAIN || errno == EWOULDBLOCK);
    return;
  }

  for (int i = 0; i < count; i++) {
    assert(messages[i].msg_len == 3);
    assert(memcmp(buffers[i], received == 0 ? "one" : "two", 3) == 0);
    received++;
  }
  if (received == 2) {
    succeed();
  }
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
