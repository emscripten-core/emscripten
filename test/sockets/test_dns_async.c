/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_dns_lookup_callback() / emscripten_dns_lookup_promise(): the
 * call variants of getaddrinfo(). Numeric addresses and errors complete at
 * once; a hostname completes after a real node:dns lookup under
 * -sNODERAWSOCKETS (or at once with a fake address without it). Either way
 * the result is delivered after the call returns, from any stack, including
 * the main thread of a plain build where getaddrinfo() itself returns
 * EAI_AGAIN.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/promise.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int stage;
int returned;

struct addrinfo hints = { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
struct addrinfo h4 = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };

void check_v4(struct addrinfo* res, const char* addr) {
  int n = 0;
  for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
    assert(ai->ai_socktype == SOCK_STREAM);
    assert(ai->ai_protocol == IPPROTO_TCP);
    assert(ai->ai_family == AF_INET);
    struct sockaddr_in* sin = (struct sockaddr_in*)ai->ai_addr;
    assert(ai->ai_addrlen == sizeof(*sin));
    assert(ntohs(sin->sin_port) == 80);
    if (!addr || sin->sin_addr.s_addr == inet_addr(addr)) n++;
  }
  assert(n >= 1);
}

void check_localhost(struct addrinfo* res) {
#ifdef REAL_DNS
  check_v4(res, "127.0.0.1");
#else
  check_v4(res, NULL);
#endif
  freeaddrinfo(res);
}

em_promise_result_t fail(void** result, void* data, void* value) {
  assert(0 && "lookup promise rejected");
}

em_promise_result_t on_resolved(void** result, void* data, void* value) {
  assert(stage++ == 3);
  check_localhost(value);
  printf("done\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  exit(0);
#endif
  return EM_PROMISE_FULFILL;
}

void on_host(bool err, intptr_t result, void* arg) {
  assert(stage++ == 2);
  assert(!err);
  assert(arg == &stage);
  assert(result > 0);
  check_localhost((struct addrinfo*)result);

  // The same lookup as a promise.
  em_promise_t p = emscripten_dns_lookup_promise("localhost", "80", &h4);
  em_promise_t next = emscripten_promise_then(p, on_resolved, fail, NULL);
  emscripten_promise_destroy(p);
  emscripten_promise_destroy(next);
}

void on_error(bool err, intptr_t result, void* arg) {
  assert(stage++ == 1);
  assert(!err);
  assert(result == EAI_SERVICE);

  // A hostname: pending until node:dns answers.
  emscripten_dns_lookup_callback("localhost", "80", &h4, on_host, &stage);
#if defined(REAL_DNS) && !defined(__EMSCRIPTEN_PTHREADS__)
  // Where the stack cannot wait, the synchronous form says so.
  struct addrinfo* res;
  assert(getaddrinfo("localhost", "80", &h4, &res) == EAI_AGAIN);
#endif
}

void on_numeric(bool err, intptr_t result, void* arg) {
  // A synchronous completion is still called back after the return.
  assert(stage++ == 0);
  assert(!err);
  assert(returned);
  struct addrinfo* res = (struct addrinfo*)result;
  check_v4(res, "10.9.8.7");
  assert(!res->ai_next);
  freeaddrinfo(res);

  // An error, as the (negative) EAI_* code.
  emscripten_dns_lookup_callback("10.9.8.7", "http", &hints, on_error, NULL);
}

int main() {
  // The synchronous form completes for a numeric address and an error.
  struct addrinfo* res = NULL;
  assert(getaddrinfo("10.9.8.7", "80", &hints, &res) == 0);
  check_v4(res, "10.9.8.7");
  freeaddrinfo(res);
  assert(getaddrinfo("10.9.8.7", "http", &hints, &res) == EAI_SERVICE);

  // A pending callback holds the runtime.
#ifndef __EMSCRIPTEN_PTHREADS__
  assert(!emscripten_runtime_keepalive_check());
#endif
  emscripten_dns_lookup_callback("10.9.8.7", "80", &hints, on_numeric, NULL);
  assert(emscripten_runtime_keepalive_check());
  returned = 1;
  return 0;
}
