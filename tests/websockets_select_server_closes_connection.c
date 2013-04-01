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
  static char readbuf[1024];
  static int readPos = 0;
  
  fd_set sett;
  FD_ZERO(&sett);
  FD_SET(SocketFD, &sett);
  
  if( readPos < 7 ){
    // still reading
    int selectRes = select(64, &sett, NULL, NULL, NULL);
    
    if( selectRes == 0 )
      return;
    
    if( selectRes == -1 ){
      perror( "Connection to websocket server failed" );
      exit(EXIT_FAILURE);
    }
    if( selectRes > 0 ){
      assert(FD_ISSET(SocketFD, &sett));
      
      int bytesRead = recv( SocketFD, readbuf+readPos, 7-readPos, 0 );
      readPos += bytesRead;
    }
  } else {
    // here the server should have closed the connection
    int selectRes = select(64, &sett, NULL, NULL, NULL);
    
    if( selectRes == 0 )
      return;
    
    if( selectRes == -1 ){
      perror( "Connection to websocket server failed as expected" );
      int result = 266;
      REPORT_RESULT();
      emscripten_cancel_main_loop();
      done = 1;
    } 
    
    if( selectRes > 0 ){
      printf( "Error: socket should not show up on select call anymore.\n" );
      exit(EXIT_FAILURE);
    }     
  }
  
  return;
}

// Scenario: the server sends data and closes the connection after 7 bytes. 
// This test should provoke the situation in which the underlying 
// tcp connection has been torn down already but there is still data 
// in the inQueue. The select call has to succeed as long the queue
// still contains data and only then start to throw errors.
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

