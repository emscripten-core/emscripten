// This test verifies that calling send() back to back right after calling
// connect() succeeds.

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <emscripten/html5.h>
#include <string.h>
#include <stdio.h>

int sock;

EM_BOOL wait_for_recv(double d, void *u) {
  // Poll read from the socket to see what we have received
  char buf[1024] = {};
  recv(sock, buf, sizeof(buf)-1, 0);
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
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(8089)
  };
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
  const char *msg = "Hello";
  ssize_t bytes = send(sock, msg, strlen(msg), 0);
  if (bytes != strlen(msg)) {
    printf("send() failed to send %d bytes. Return value: %d\n", (int)strlen(msg), (int)bytes);
    return bytes;
  }

  // Asynchronously wait until we get the message echoed back
  emscripten_set_timeout_loop(wait_for_recv, 0, 0);
  return 0;
}
