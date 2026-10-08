/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * emscripten_dns_lookup_fd() / emscripten_dns_lookup_promise(): the call
 * variants of getaddrinfo(). Numeric addresses and errors complete at once; a
 * hostname completes after a real node:dns lookup under -sNODERAWSOCKETS (or
 * at once with a fake address without it). The fd is awaited with poll() from
 * any stack, including the main thread of a plain build where getaddrinfo()
 * itself returns EAI_AGAIN.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/promise.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int fd;
int polls;

struct addrinfo hints = { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };

// From a pthread the lookup runs on the main thread, so even an immediate
// result is readable only once that has happened: wait for it.
#ifdef __EMSCRIPTEN_PTHREADS__
#define POLL_TIMEOUT -1
#else
#define POLL_TIMEOUT 0
#endif

int readable(int fd) {
  struct pollfd p = { .fd = fd, .events = POLLIN };
  int n = poll(&p, 1, POLL_TIMEOUT);
  assert(n == 0 || (n == 1 && (p.revents & POLLIN)));
  return n;
}

intptr_t take(int fd) {
  intptr_t r;
  assert(read(fd, &r, sizeof r) == sizeof r);
  assert(close(fd) == 0);
  return r;
}

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

em_promise_result_t fail(void** result, void* data, void* value) {
  assert(0 && "lookup promise rejected");
}

em_promise_result_t on_resolved(void** result, void* data, void* value) {
  struct addrinfo* res = value;
#ifdef REAL_DNS
  check_v4(res, "127.0.0.1");
#else
  check_v4(res, NULL);
#endif
  freeaddrinfo(res);
  printf("done\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  exit(0);
#endif
  return EM_PROMISE_FULFILL;
}

void finish(void* arg) {
  if (!readable(fd)) {
    polls++;
#ifdef __EMSCRIPTEN_PTHREADS__
    // A pthread can simply block on the fd.
    struct pollfd p = { .fd = fd, .events = POLLIN };
    assert(poll(&p, 1, -1) == 1 && (p.revents & POLLIN));
#else
    // The main thread cannot: retry on the next turn of the event loop.
    emscripten_set_timeout(finish, 0, NULL);
    return;
#endif
  }
  intptr_t r = take(fd);
  assert(r > 0);
  struct addrinfo* res = (struct addrinfo*)r;
#ifdef REAL_DNS
#ifndef __EMSCRIPTEN_PTHREADS__
  assert(polls > 0);
#endif
  check_v4(res, "127.0.0.1");
#else
  check_v4(res, NULL);
#endif
  freeaddrinfo(res);

  // The same lookup as a promise.
  struct addrinfo h4 = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
  em_promise_t p = emscripten_dns_lookup_promise("localhost", "80", &h4);
  em_promise_t next = emscripten_promise_then(p, on_resolved, fail, NULL);
  emscripten_promise_destroy(p);
  emscripten_promise_destroy(next);
}

int main() {
  // A numeric address needs no lookup: readable on return (on the main
  // thread).
  fd = emscripten_dns_lookup_fd("10.9.8.7", "80", &hints);
  assert(readable(fd));
  struct addrinfo* res = (struct addrinfo*)take(fd);
  check_v4(res, "10.9.8.7");
  assert(!res->ai_next);
  freeaddrinfo(res);

  // So does an error, as the (negative) EAI_* code.
  fd = emscripten_dns_lookup_fd("10.9.8.7", "http", &hints);
  assert(readable(fd));
  assert(take(fd) == EAI_SERVICE);

  // And the synchronous form completes for both.
  res = NULL;
  assert(getaddrinfo("10.9.8.7", "80", &hints, &res) == 0);
  check_v4(res, "10.9.8.7");
  freeaddrinfo(res);
  assert(getaddrinfo("10.9.8.7", "http", &hints, &res) == EAI_SERVICE);

  // Closing while a real lookup is still pending drops its result and
  // releases its runtime hold.
  struct addrinfo h4 = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
#ifndef __EMSCRIPTEN_PTHREADS__
  assert(!emscripten_runtime_keepalive_check());
#endif
  fd = emscripten_dns_lookup_fd("localhost", "80", &h4);
#if defined(REAL_DNS) && !defined(__EMSCRIPTEN_PTHREADS__)
  assert(emscripten_runtime_keepalive_check());
#endif
  assert(close(fd) == 0);
#ifndef __EMSCRIPTEN_PTHREADS__
  assert(!emscripten_runtime_keepalive_check());
#endif

  // A hostname: pending until node:dns answers. (On a pthread the proxied
  // calls give the main thread's loop turns in between, so it may already
  // have completed.)
  fd = emscripten_dns_lookup_fd("localhost", "80", &h4);
#if defined(REAL_DNS) && !defined(__EMSCRIPTEN_PTHREADS__)
  assert(!readable(fd));
  // Where the stack cannot wait, the synchronous form says so.
  assert(getaddrinfo("localhost", "80", &h4, &res) == EAI_AGAIN);
#endif
  finish(NULL);
  return 0;
}
