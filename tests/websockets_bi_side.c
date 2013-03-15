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
#if EMSCRIPTEN
#include <emscripten.h>
#endif

#define EXPECTED_BYTES 5

int main(void)
{
  struct sockaddr_in stSockAddr;
  int Res;
#if !TEST_DGRAM
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  int SocketFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif

  if (-1 == SocketFD)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(stSockAddr));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(SOCKK);
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

  printf("connect..\n");

  if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  printf("send..\n");

  char data[] = "hello from the other siide\n";
  send(SocketFD, data, sizeof(data), 0);

  printf("stall..\n");

  //int bytes;
  //while (1) ioctl(SocketFD, FIONREAD, &bytes);

  return EXIT_SUCCESS;
}

