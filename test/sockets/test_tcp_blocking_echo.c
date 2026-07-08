/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Self-contained TCP loopback echo using only BLOCKING socket calls: no
 * O_NONBLOCK, no select/poll, no emscripten_set_main_loop. A listener and a
 * client live in one process; each blocking connect/accept/recv/send suspends
 * the wasm stack (JSPI) - or, under PROXY_TO_PTHREAD, blocks the worker while
 * the main thread's event loop services the sockets - until it can complete.
 * This is plain POSIX and also builds and runs natively against the host stack.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
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
  addr.sin_port = htons(0); // ephemeral
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(listen_fd, 4) == 0);

  socklen_t la_len = sizeof(addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &la_len) == 0);
  assert(ntohs(addr.sin_port) != 0);

  // Blocking connect: suspends until the loopback handshake completes.
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);

  // Blocking accept: the connection queued during connect(), so this returns
  // the server-side peer (blocking until one is pending in the general case).
  struct sockaddr_in ca;
  socklen_t ca_len = sizeof(ca);
  int peer_fd = accept(listen_fd, (struct sockaddr*)&ca, &ca_len);
  assert(peer_fd >= 0);

  // Blocking send then blocking recv, in both directions.
  assert(send(client_fd, "ping", 4, 0) == 4);

  char buf[4];
  assert(recv(peer_fd, buf, sizeof(buf), 0) == 4);
  assert(memcmp(buf, "ping", 4) == 0);
  assert(send(peer_fd, "pong", 4, 0) == 4);

  assert(recv(client_fd, buf, sizeof(buf), 0) == 4);
  assert(memcmp(buf, "pong", 4) == 0);

  // Same again over read()/write() (fd_read/fd_write), which block on the
  // socket via the same suspend path as recv/send.
  assert(write(client_fd, "ping", 4) == 4);
  assert(read(peer_fd, buf, sizeof(buf)) == 4);
  assert(memcmp(buf, "ping", 4) == 0);
  assert(write(peer_fd, "pong", 4) == 4);
  assert(read(client_fd, buf, sizeof(buf)) == 4);
  assert(memcmp(buf, "pong", 4) == 0);

  close(client_fd);
  close(peer_fd);
  close(listen_fd);
  printf("done\n");
  return 0;
}
