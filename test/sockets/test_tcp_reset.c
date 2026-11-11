/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A peer reset (SO_LINGER 0, then close -> RST) must surface from recv() as
 * ECONNRESET rather than as a clean EOF.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(listen_fd, 4) == 0);
  socklen_t l = sizeof(addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &l) == 0);

  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);

  struct pollfd p = { .fd = listen_fd, .events = POLLIN };
  assert(poll(&p, 1, -1) == 1);
  int peer_fd = accept(listen_fd, NULL, NULL);
  assert(peer_fd >= 0);

  struct linger lg = { .l_onoff = 1, .l_linger = 0 };
  assert(setsockopt(peer_fd, SOL_SOCKET, SO_LINGER, &lg, sizeof(lg)) == 0);
  close(peer_fd);

  p.fd = client_fd;
  assert(poll(&p, 1, -1) == 1);
  char buf[8];
  assert(recv(client_fd, buf, sizeof(buf), 0) == -1);
  assert(errno == ECONNRESET);

  close(client_fd);
  close(listen_fd);
  printf("done\n");
  return 0;
}
