/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * With -sNODERAWSOCKETS under JSPI, getaddrinfo() of a real public hostname
 * blocks on the node:dns lookup by suspending the wasm stack, and resolves
 * directly - no EAI_AGAIN + async prewarm + retry needed. This resolves over
 * the real network.
 */

#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

static const char* HOST = "google.com";

int main(void) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* res = NULL;
  int err = getaddrinfo(HOST, NULL, &hints, &res);
  if (err != 0) {
    printf("DNS JSPI FAIL: getaddrinfo err=%d\n", err);
    return 1;
  }
  assert(res);
  freeaddrinfo(res);

  printf("DNS JSPI PASS\n");
  return 0;
}
