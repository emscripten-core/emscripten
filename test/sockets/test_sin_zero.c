/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <assert.h>

int main() {
  struct sockaddr_in addr;

  int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  connect(s, (struct sockaddr *)&addr, sizeof(addr));

  struct sockaddr_storage adr_inet;
  socklen_t len_inet = sizeof adr_inet;
  memset(&adr_inet, 1, sizeof(adr_inet));
  getsockname(s, (struct sockaddr *)&adr_inet, &len_inet);

  struct sockaddr_in *adr_inet4 = (struct sockaddr_in *)&adr_inet;
  for (int i = 0; i < 8; i++) {
    assert(adr_inet4->sin_zero[i] == 0);
  }
  shutdown(s, SHUT_RDWR);

  puts("success");

  return EXIT_SUCCESS;
}
