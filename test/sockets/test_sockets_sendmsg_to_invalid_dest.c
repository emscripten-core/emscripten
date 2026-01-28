// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <arpa/inet.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <ifaddrs.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int sock;

bool wait_for_recv(double d, void* u) {
  // Poll read from the socket to see what we have received
  char buf[1024] = {};
  recv(sock, buf, sizeof(buf) - 1, 0);
  if (strlen(buf) > 0) {
    printf("%s\n", buf);
    if (!strcmp(buf, "Hello")) {
      printf("Got hello, test finished.\n");
#ifdef REPORT_RESULT
      REPORT_RESULT(0);
#endif
    }
  }
  return EM_TRUE;
}

int main() {
  // Connect socket to a WebSocket echo server
  struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(8089)};
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    printf("socket() failed to error %d\n", sock);
    return sock;
  }

  // Connect to echo server.
  int error = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  if (error) {
    printf("connect() failed to error %d\n", error);
    return error;
  }

  // Immediately send a message back-to-back from connecting to the socket
  const char* msg = "Hello";
  ssize_t bytes = send(sock, msg, strlen(msg), 0);
  if (bytes != strlen(msg)) {
    printf("send() failed to send %d bytes. Return value: %d\n",
           (int)strlen(msg),
           (int)bytes);
    return bytes;
  }

  // This shouldn't throw an exception in `sendmsg` implementation.
  // [see GH-23046]
  struct ifaddrs* ifaddrs = NULL;
  getifaddrs(&ifaddrs);

  // Asynchronously wait until we get the message echoed back
  emscripten_set_timeout_loop(wait_for_recv, 0, 0);
  return 0;
}
