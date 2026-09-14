/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * getsockname() immediately after a non-blocking connect() on an unbound
 * client must report the kernel-assigned ephemeral source port: the kernel
 * assigns it synchronously at connect(), not when the connection completes.
 * The port must then stay the same once connected. This is plain POSIX and
 * also builds and runs natively, so the same code can be checked against the
 * host stack.
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

int listen_fd = -1;
int client_fd = -1;
int peer_fd = -1;
struct sockaddr_in dest;
bool connected = false;
uint16_t client_port = 0; // network order, recorded right after connect()

void set_nonblocking(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);
}

void test_success(void) {
  printf("done\n");
  if (listen_fd >= 0) close(listen_fd);
  if (client_fd >= 0) close(client_fd);
  if (peer_fd >= 0) close(peer_fd);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  exit(0);
#endif
}

void start_client(void) {
  if (client_fd >= 0) close(client_fd);
  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  set_nonblocking(client_fd);
  connected = false;
  int r = connect(client_fd, (struct sockaddr*)&dest, sizeof(dest));
  assert((r == 0 || errno == EINPROGRESS) && "connect");

  // The ephemeral source port is assigned synchronously at connect(): it must
  // be visible here, before the connection completes or any event turn runs.
  struct sockaddr_in sa;
  socklen_t sl = sizeof(sa);
  int g = getsockname(client_fd, (struct sockaddr*)&sa, &sl);
  assert(g == 0 && "getsockname after connect");
  assert(ntohs(sa.sin_port) != 0 && "ephemeral port assigned at connect()");
  client_port = sa.sin_port;
  printf("connecting from port %u\n", (unsigned)ntohs(client_port));
}

void main_loop(void) {
  fd_set fdr, fdw;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(listen_fd, &fdr);
  FD_SET(client_fd, &fdw);
  select(64, &fdr, &fdw, NULL, &tv);

  if (peer_fd < 0 && FD_ISSET(listen_fd, &fdr)) {
    peer_fd = accept(listen_fd, NULL, NULL);
    if (peer_fd >= 0) set_nonblocking(peer_fd);
  }

  if (!connected && FD_ISSET(client_fd, &fdw)) {
    int err = 0;
    socklen_t l = sizeof(err);
    getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &err, &l);
    if (err == ECONNREFUSED || err == ECONNRESET) {
      start_client();
      return;
    }
    assert(err == 0 && "connect failed");
    connected = true;

    // The name must not change when the connection completes.
    struct sockaddr_in sa;
    socklen_t sl = sizeof(sa);
    assert(getsockname(client_fd, (struct sockaddr*)&sa, &sl) == 0);
    assert(sa.sin_port == client_port && "port stable across connect completion");
    test_success();
  }
}

int main(void) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(0);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("bind");
    return 1;
  }
  if (listen(listen_fd, 4) != 0) {
    perror("listen");
    return 1;
  }

  struct sockaddr_in la;
  socklen_t ll = sizeof(la);
  if (getsockname(listen_fd, (struct sockaddr*)&la, &ll) != 0) {
    perror("getsockname");
    return 1;
  }
  set_nonblocking(listen_fd);

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = la.sin_port;
  inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);
  start_client();

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
