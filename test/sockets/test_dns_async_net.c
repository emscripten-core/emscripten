/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Asynchronous getaddrinfo over the real network with -sNODERAWSOCKETS: a real
 * public hostname has no synchronous resolution (EAI_AGAIN), then resolves via
 * emscripten_dns_lookup_async(), whose result is delivered as an addrinfo
 * payload. This hits the real network (like test_getaddrinfo).
 */

#include <assert.h>
#include <emscripten.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static const char* HOST = "google.com";

static int lookup_fd = -1;

static void fail(const char* why) {
  printf("DNS ASYNC NET FAIL: %s\n", why);
  abort();
}

static void main_loop(void) {
  fd_set fdr;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_SET(lookup_fd, &fdr);
  select(lookup_fd + 1, &fdr, NULL, NULL, &tv);
  if (!FD_ISSET(lookup_fd, &fdr)) {
    return; // resolution still in flight
  }

  struct addrinfo* res = NULL;
  int result = emscripten_dns_lookup_result(lookup_fd, &res);
  close(lookup_fd);
  if (result != 0) fail("async lookup failed");
  assert(res && res->ai_addr);
  freeaddrinfo(res);

  printf("DNS ASYNC NET PASS\n");
  emscripten_cancel_main_loop();
}

int main(void) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // No synchronous DNS without JSPI: the name is EAI_AGAIN up front.
  struct addrinfo* res = NULL;
  if (getaddrinfo(HOST, NULL, &hints, &res) != EAI_AGAIN) fail("host should be EAI_AGAIN");

  lookup_fd = emscripten_dns_lookup_async(HOST, NULL, &hints);
  if (lookup_fd < 0) fail("async lookup did not return an fd");

  emscripten_set_main_loop(main_loop, 0, 0);
  return 0;
}
