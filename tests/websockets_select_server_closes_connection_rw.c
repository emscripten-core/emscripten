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
  static int state = 0;
  static char writebuf[] = "01234567890123456789";
  static int writePos = 0;
  static char readbuf[1024];
  static int readPos = 0;
  int selectRes;
  ssize_t transferAmount;
  fd_set sett;
  
  
  switch( state ){
    case 0:
      // writing 10 bytes to the server
      
      // the socket in the read file descriptors has to result in a 0 return value
      // because the connection exists, but there is no data yet
      FD_ZERO( &sett );
      FD_SET(SocketFD, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if( selectRes != 0 ){
        printf( "case 0: read select != 0\n" );
        exit(EXIT_FAILURE);
      }
      
      // the socket in the write file descriptors has to result in either a 0 or 1
      // the connection either is setting up or is established and writing is possible
      FD_ZERO( &sett );
      FD_SET(SocketFD, &sett);
      selectRes = select(64, NULL, &sett, NULL, NULL);
      if( selectRes == -1 ){
        printf( "case 0: write select == -1\n" );
        exit(EXIT_FAILURE);
      }
      if( selectRes == 0 ){
        return;
      }
      
      // send a single byte
      transferAmount = send( SocketFD, writebuf+writePos, 1, 0 );
      writePos += transferAmount;
   
      // after 10 bytes switch to next state
      if( writePos >= 10 ){
        state = 1;
      }
      break;
      
    case 1:
      // wait until we can read one byte to make sure the server
      // has sent the data and then closed the connection
      FD_ZERO( &sett );
      FD_SET(SocketFD, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if( selectRes == -1 ){
        printf( "case 1: read selectRes == -1\n" );
        exit(EXIT_FAILURE);
      }
      if( selectRes == 0 )
        return;

      // read a single byte
      transferAmount = recv( SocketFD, readbuf+readPos, 1, 0 );
      readPos += transferAmount;
   
      // if successfully reading 1 byte, switch to next state
      if( readPos >= 1 ){
        state = 2;
      }
      break;
    
    case 2:
      // calling select with the socket in the write file descriptors has
      // to fail because the tcp network connection is already down
      FD_ZERO( &sett );
      FD_SET(SocketFD, &sett);
      selectRes = select(64, NULL, &sett, NULL, NULL);
      if( selectRes != -1 ){
        printf( "case 2: write selectRes != -1\n" );
        exit(EXIT_FAILURE);
      }

      // calling select with the socket in the read file descriptors 
      // has to succeed because there is still data in the inQueue
      FD_ZERO( &sett );
      FD_SET(SocketFD, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if( selectRes != 1 ){
        printf( "case 2: read selectRes != 1\n" );
        exit(EXIT_FAILURE);
      }
      if( selectRes == 0 )
        return;
      
      // read a single byte
      transferAmount = recv( SocketFD, readbuf+readPos, 1, 0 );
      readPos += transferAmount;
      
      // with 10 bytes read the inQueue is empty => switch state
      if( readPos >= 10 ){
        state = 3;
      }
      break;
      
    case 3:
      // calling select with the socket in the read file descriptors 
      // now also has to fail as the inQueue is empty
      FD_ZERO( &sett );
      FD_SET(SocketFD, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if( selectRes != -1 ){
        printf( "case 3: read selectRes != -1\n" );
        exit(EXIT_FAILURE);
      }
      
      // report back success, the 266 is just an arbitrary value without 
      // deeper meaning
      int result = 266;
      REPORT_RESULT();
      break;
      
    default:
      printf( "Impossible state!\n" );
      exit(EXIT_FAILURE);
      break;
  }
  
  return;
}

// This test checks for an intended asymmetry in the behavior of the select function.
// Scenario: the client sends data to the server. After 10 received bytes the 
// server sends 10 bytes on its own and immediately afterwards closes the connection.
// This mimics a typical connect-request-response-disconnect situation.
// After the server closed the connection select calls with the socket in the write file 
// descriptors have to fail as the tcp connection is already down and there is no way 
// anymore to send data. 
// Select calls with the socket in the read file descriptor list still have to succeed 
// as there are still 10 bytes to read from the inQueue. So, for the same socket the 
// select call behaves differently depending on whether the socket is listed in the
// read or write file descriptors.
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
    8999
#else
    8998
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

