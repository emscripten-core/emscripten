/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(sockfd >= 0);
  close(sockfd);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(sockfd >= 0);
  close(sockfd);

  errno = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
  assert(sockfd == -1);
  assert(errno == EPROTONOSUPPORT);

  errno = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
  assert(sockfd == -1);
  assert(errno == EPROTONOSUPPORT);

  puts("success");

  return EXIT_SUCCESS;
}
