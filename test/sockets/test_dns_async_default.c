/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * The async getaddrinfo API is available without -sNODERAWSOCKETS too: there it
 * resolves synchronously (the same fake address getaddrinfo() returns) and the
 * fd is simply readable on the next turn, so integration code need not branch on
 * the backend.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static int lookup_fd = -1;

static void fail(const char* why) {
  printf("DNS ASYNC DEFAULT FAIL: %s\n", why);
  abort();
}

static void main_loop(void) {
  fd_set fdr;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_SET(lookup_fd, &fdr);
  select(lookup_fd + 1, &fdr, NULL, NULL, &tv);
  if (!FD_ISSET(lookup_fd, &fdr)) {
    return;
  }

  struct addrinfo* res = NULL;
  int result = emscripten_dns_lookup_result(lookup_fd, &res);
  close(lookup_fd);
  if (result != 0) fail("async lookup failed");
  assert(res && res->ai_addr);
  if (res->ai_socktype != SOCK_STREAM) fail("async result lost the requested socktype");
  freeaddrinfo(res);

  printf("DNS ASYNC DEFAULT PASS\n");
  emscripten_cancel_main_loop();
}

int main(void) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  lookup_fd = emscripten_dns_lookup_async("example.com", NULL, &hints);
  if (lookup_fd < 0) fail("async lookup did not return an fd");

  emscripten_set_main_loop(main_loop, 0, 0);
  return 0;
}
