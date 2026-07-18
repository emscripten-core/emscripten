/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Socket metadata and SOL_SOCKET options under the node:net backend, on a
 * freshly created (unconnected) socket: fstat() reports a socket (S_ISSOCK),
 * SO_TYPE reports the socket type, and SO_LINGER round-trips a struct linger.
 * Plain POSIX, also runs natively.
 */

#include <assert.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
  int tcp = socket(AF_INET, SOCK_STREAM, 0);
  assert(tcp >= 0);
  int udp = socket(AF_INET, SOCK_DGRAM, 0);
  assert(udp >= 0);

  // fstat reports a socket for both types.
  struct stat st;
  assert(fstat(tcp, &st) == 0);
  assert(S_ISSOCK(st.st_mode));
  assert(fstat(udp, &st) == 0);
  assert(S_ISSOCK(st.st_mode));

  // SO_TYPE reports the type the socket was created with.
  int type = 0;
  socklen_t len = sizeof(type);
  assert(getsockopt(tcp, SOL_SOCKET, SO_TYPE, &type, &len) == 0);
  assert(type == SOCK_STREAM);
  type = 0;
  len = sizeof(type);
  assert(getsockopt(udp, SOL_SOCKET, SO_TYPE, &type, &len) == 0);
  assert(type == SOCK_DGRAM);

  // SO_LINGER round-trips a struct linger.
  struct linger set = {.l_onoff = 1, .l_linger = 5};
  assert(setsockopt(tcp, SOL_SOCKET, SO_LINGER, &set, sizeof(set)) == 0);
  struct linger got;
  memset(&got, 0, sizeof(got));
  len = sizeof(got);
  assert(getsockopt(tcp, SOL_SOCKET, SO_LINGER, &got, &len) == 0);
  assert(len == sizeof(got));
  assert(got.l_onoff == 1);
  assert(got.l_linger == 5);

  // TCP_MAXSEG reports the RFC 879 default MSS before a connection is made.
  int mss = 0;
  len = sizeof(mss);
  assert(getsockopt(tcp, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) == 0);
  assert(mss == 536);

  close(tcp);
  close(udp);
  printf("done\n");
  return 0;
}
