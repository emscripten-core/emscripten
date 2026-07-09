/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Exercises the -sNODERAWSOCKETS DNS path. getaddrinfo() resolves numeric and
 * /etc/hosts names synchronously (the latter read through emscripten's FS) and
 * returns EAI_AGAIN for a real hostname. emscripten_dns_lookup_async() is the
 * asynchronous getaddrinfo: it takes the same node/service/hints and returns a
 * pollable fd; emscripten_dns_lookup_result() then yields the addrinfo payload.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

static int lookup_fd = -1;

static void fail(const char* why) {
  printf("DNS ASYNC FAIL: %s\n", why);
  abort();
}

// getaddrinfo() of an AF_INET hostname, returning its first address (or *err).
static unsigned ipv4_of(const char* name, int* err_out) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo* res = NULL;
  int err = getaddrinfo(name, NULL, &hints, &res);
  if (err_out) *err_out = err;
  if (err != 0) return 0;
  unsigned addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  return addr;
}

static void main_loop(void) {
  fd_set fdr;
  struct timeval tv = {0};
  FD_ZERO(&fdr);
  FD_SET(lookup_fd, &fdr);
  select(lookup_fd + 1, &fdr, NULL, NULL, &tv);
  if (!FD_ISSET(lookup_fd, &fdr)) {
    return; // resolution still pending
  }

  // The result is delivered directly as an addrinfo payload, in the same format
  // getaddrinfo() produces (and freed the same way).
  struct addrinfo* res = NULL;
  int result = emscripten_dns_lookup_result(lookup_fd, &res);
  close(lookup_fd);
  if (result != 0) fail("async lookup failed");
  assert(res);
  if (res->ai_socktype != SOCK_STREAM) fail("async result lost the requested socktype");
  unsigned addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  if (addr != htonl(INADDR_LOOPBACK)) fail("localhost did not resolve to 127.0.0.1");

  printf("DNS ASYNC PASS\n");
  emscripten_cancel_main_loop();
}

int main(void) {
  // Seed /etc/hosts (through emscripten's FS) with names node:dns could never
  // resolve, including one mapped to multiple addresses.
  mkdir("/etc", 0777);
  FILE* f = fopen("/etc/hosts", "w");
  assert(f);
  fputs("# test hosts\n"
        "10.1.2.3   statichost.test\n"
        "192.0.2.1  multi.test\n"
        "192.0.2.2  multi.test\n",
        f);
  fclose(f);

  // /etc/hosts resolves synchronously through getaddrinfo.
  int err = 0;
  unsigned static_addr = ipv4_of("statichost.test", &err);
  if (err != 0) fail("static host not resolved from /etc/hosts");
  if (static_addr != inet_addr("10.1.2.3")) fail("static host wrong address");

  // A name with several addresses comes back as an addrinfo linked list, freed
  // as a whole by freeaddrinfo.
  struct addrinfo mhints = {0};
  mhints.ai_family = AF_INET;
  mhints.ai_socktype = SOCK_STREAM;
  struct addrinfo* mres = NULL;
  if (getaddrinfo("multi.test", NULL, &mhints, &mres) != 0) fail("multi host not resolved");
  int count = 0, seen1 = 0, seen2 = 0;
  for (struct addrinfo* ai = mres; ai; ai = ai->ai_next) {
    unsigned a = ((struct sockaddr_in*)ai->ai_addr)->sin_addr.s_addr;
    if (a == inet_addr("192.0.2.1")) seen1 = 1;
    if (a == inet_addr("192.0.2.2")) seen2 = 1;
    count++;
  }
  freeaddrinfo(mres);
  if (count != 2 || !seen1 || !seen2) fail("multi host did not return both addresses");

  // A real hostname not in /etc/hosts has no synchronous resolution.
  ipv4_of("localhost", &err);
  if (err != EAI_AGAIN) fail("real hostname should be EAI_AGAIN");

  // Resolve it asynchronously; the result arrives via the pollable fd.
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  lookup_fd = emscripten_dns_lookup_async("localhost", NULL, &hints);
  if (lookup_fd < 0) fail("async lookup did not return an fd");

  emscripten_set_main_loop(main_loop, 0, 0);
  return 0;
}
