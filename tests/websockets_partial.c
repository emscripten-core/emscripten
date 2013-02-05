#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <assert.h>
#if EMSCRIPTEN
#include <emscripten.h>
#endif

int SocketFD;
int done = 0;
int sum = 0;

void iter(void *arg) {
  char buffer[1024];
  char packetLength;
  int n;
  int i;

  if (done) {
    return;
  }

  n = recv(SocketFD, buffer, 1, 0);

  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return; //try again
    }

    fprintf(stderr, "unexcepted end of data");
    exit(EXIT_FAILURE);
  }

  if (n != 1) {
    fprintf(stderr, "should read 1 byte");
    exit(EXIT_FAILURE);
  }

  packetLength = buffer[0];
  n = recv(SocketFD, buffer, packetLength, 0);

  printf("got %d,%d\n", n, packetLength);

  if (n != packetLength) {
    fprintf(stderr, "lost packet data, expected: %d readed: %d", packetLength, n);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < packetLength; ++i) {
    if (buffer[i] != i+1) {
      fprintf(stderr, "packet corrupted, expected: %d, actual: %d", i+1, buffer[i]);
      exit(EXIT_FAILURE);
    }

    sum += buffer[i];
  }

  if (packetLength == buffer[0]) { // \x01\x01 - end marker
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    done = 1;

    #if EMSCRIPTEN
        printf("sum: %d\n", sum);
        int result = sum;
        REPORT_RESULT();
    #endif
  }
}

int main(void)
{
  struct sockaddr_in stSockAddr;
  int Res;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(stSockAddr));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(
#if EMSCRIPTEN
    8991
#else
    8990
#endif
  );
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res) {
    perror("error: first parameter is not a valid address family");
    close(SocketFD);
    exit(EXIT_FAILURE);
  } else if (0 == Res) {
    perror("char string (second parameter does not contain valid ipaddress)");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);

  }

#if EMSCRIPTEN
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (!done) iter(NULL);
#endif

  return EXIT_SUCCESS;
}

