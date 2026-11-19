/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Socket metadata and SOL_SOCKET options under the node:net backend, on a
 * freshly created (unconnected) socket: fstat() reports a socket (S_ISSOCK),
 * the read-only SO_TYPE/SO_DOMAIN/SO_PROTOCOL/SO_ACCEPTCONN report socket
 * identity, SO_LINGER and the timeouts round-trip their structs, and unknown
 * options fail symmetrically with ENOPROTOOPT. Plain POSIX, also runs natively.
 */

#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int get_int(int fd, int level, int opt) {
  int val = -1;
  socklen_t len = sizeof(val);
  assert(getsockopt(fd, level, opt, &val, &len) == 0);
  assert(len == sizeof(val));
  return val;
}

void set_int(int fd, int level, int opt, int val) {
  assert(setsockopt(fd, level, opt, &val, sizeof(val)) == 0);
}

void check_timeo(int fd, int opt) {
  struct timeval tv, got;
  socklen_t len = sizeof(got);
  // Unset reads as zero (no timeout).
  memset(&got, 0xff, sizeof(got));
  assert(getsockopt(fd, SOL_SOCKET, opt, &got, &len) == 0);
  assert(len == sizeof(got));
  assert(got.tv_sec == 0 && got.tv_usec == 0);
  // Round-trip.
  tv.tv_sec = 2;
  tv.tv_usec = 500000;
  assert(setsockopt(fd, SOL_SOCKET, opt, &tv, sizeof(tv)) == 0);
  memset(&got, 0, sizeof(got));
  len = sizeof(got);
  assert(getsockopt(fd, SOL_SOCKET, opt, &got, &len) == 0);
  assert(len == sizeof(got));
  assert(got.tv_sec == 2 && got.tv_usec == 500000);
  // Kernel validation: short optlen is EINVAL, out-of-range usec is EDOM, and
  // neither disturbs the stored value.
  assert(setsockopt(fd, SOL_SOCKET, opt, &tv, sizeof(tv) - 1) == -1 && errno == EINVAL);
  tv.tv_usec = 1000000;
  assert(setsockopt(fd, SOL_SOCKET, opt, &tv, sizeof(tv)) == -1 && errno == EDOM);
  len = sizeof(got);
  assert(getsockopt(fd, SOL_SOCKET, opt, &got, &len) == 0);
  assert(got.tv_sec == 2 && got.tv_usec == 500000);
  // Zero clears it.
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  assert(setsockopt(fd, SOL_SOCKET, opt, &tv, sizeof(tv)) == 0);
  len = sizeof(got);
  assert(getsockopt(fd, SOL_SOCKET, opt, &got, &len) == 0);
  assert(got.tv_sec == 0 && got.tv_usec == 0);
}

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

  // SO_TYPE/SO_DOMAIN/SO_PROTOCOL report what the socket was created with;
  // protocol 0 resolves to the default protocol for the type.
  assert(get_int(tcp, SOL_SOCKET, SO_TYPE) == SOCK_STREAM);
  assert(get_int(udp, SOL_SOCKET, SO_TYPE) == SOCK_DGRAM);
  assert(get_int(tcp, SOL_SOCKET, SO_DOMAIN) == AF_INET);
  assert(get_int(udp, SOL_SOCKET, SO_DOMAIN) == AF_INET);
  assert(get_int(tcp, SOL_SOCKET, SO_PROTOCOL) == IPPROTO_TCP);
  assert(get_int(udp, SOL_SOCKET, SO_PROTOCOL) == IPPROTO_UDP);
  // The read-only options reject a set.
  int type = SOCK_DGRAM;
  assert(setsockopt(tcp, SOL_SOCKET, SO_TYPE, &type, sizeof(type)) == -1 && errno == ENOPROTOOPT);
  assert(setsockopt(tcp, SOL_SOCKET, SO_ACCEPTCONN, &type, sizeof(type)) == -1 && errno == ENOPROTOOPT);

  // SO_ACCEPTCONN flips once the socket is listening.
  assert(get_int(tcp, SOL_SOCKET, SO_ACCEPTCONN) == 0);
  int listener = socket(AF_INET, SOCK_STREAM, 0);
  assert(listener >= 0);
  struct sockaddr_in sa = {.sin_family = AF_INET, .sin_port = 0, .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
  assert(bind(listener, (struct sockaddr*)&sa, sizeof(sa)) == 0);
  assert(get_int(listener, SOL_SOCKET, SO_ACCEPTCONN) == 0);
  assert(listen(listener, 1) == 0);
  assert(get_int(listener, SOL_SOCKET, SO_ACCEPTCONN) == 1);
  close(listener);

  // Receive/send timeouts store and read back a struct timeval.
  check_timeo(tcp, SO_RCVTIMEO);
  check_timeo(tcp, SO_SNDTIMEO);
  check_timeo(udp, SO_RCVTIMEO);

  // SO_REUSEADDR reads back what was set.
  assert(get_int(tcp, SOL_SOCKET, SO_REUSEADDR) == 0);
  set_int(tcp, SOL_SOCKET, SO_REUSEADDR, 1);
  assert(get_int(tcp, SOL_SOCKET, SO_REUSEADDR) == 1);

  // An unknown option is rejected symmetrically.
  int unknown = 1;
  socklen_t len = sizeof(unknown);
  assert(setsockopt(tcp, SOL_SOCKET, 200, &unknown, sizeof(unknown)) == -1 && errno == ENOPROTOOPT);
  assert(getsockopt(tcp, SOL_SOCKET, 200, &unknown, &len) == -1 && errno == ENOPROTOOPT);
  assert(setsockopt(tcp, IPPROTO_TCP, 200, &unknown, sizeof(unknown)) == -1 && errno == ENOPROTOOPT);
  assert(getsockopt(tcp, IPPROTO_TCP, 200, &unknown, &len) == -1 && errno == ENOPROTOOPT);

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
