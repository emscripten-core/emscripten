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

#define EXPECTED_BYTES 5

int SocketFD;

int done = 0;

void iter(void *arg) {  
  fd_set sett;
  FD_ZERO(&sett);
  FD_SET(SocketFD, &sett);
  
  // The error should happen here
  int select_says_yes = select(64, &sett, NULL, NULL, NULL);
  if( select_says_yes == -1 ){
    printf( "Connection to websocket server failed as expected." );
    perror( "Error message" );
    int result = 266;
    REPORT_RESULT();
    done = 1;
  }

  assert(!select_says_yes);
  done = 1;
}

// This is for testing a websocket connection to a closed server port.
// The connect call will succeed (due to the asynchronous websocket
// behavior) but once the underlying websocket system realized that 
// the connection cannot be established, the next select call will fail.
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
    8995
#else
    8994
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

  // This call should succeed (even if the server port is closed)
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

