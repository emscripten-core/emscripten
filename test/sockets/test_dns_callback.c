/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * The fd returned by emscripten_dns_lookup_async() signals completion not only
 * by becoming readable, but also by delivering the socket message callback
 * registered with emscripten_set_socket_message_callback(). This drives the
 * lookup purely through that callback, with no poll/select.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

static int lookup_fd = -1;

static void fail(const char* why) {
  printf("DNS CALLBACK FAIL: %s\n", why);
  abort();
}

static void on_message(int fd, void* userData) {
  if (fd != lookup_fd) return; // not our lookup

  struct addrinfo* res = NULL;
  int result = emscripten_dns_lookup_result(lookup_fd, &res);
  close(lookup_fd);
  if (result != 0) fail("async lookup failed");
  assert(res);
  unsigned addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  if (addr != htonl(INADDR_LOOPBACK)) fail("localhost did not resolve to 127.0.0.1");

  printf("DNS CALLBACK PASS\n");
}

int main(void) {
  emscripten_set_socket_message_callback(NULL, on_message);

  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  lookup_fd = emscripten_dns_lookup_async("localhost", NULL, &hints);
  if (lookup_fd < 0) fail("async lookup did not return an fd");

  // The pending node:dns lookup keeps the runtime alive; the message callback
  // fires on completion.
  return 0;
}
