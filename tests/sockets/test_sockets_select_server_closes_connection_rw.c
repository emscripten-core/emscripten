#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#if EMSCRIPTEN
#include <emscripten.h>
#endif

#define EXPECTED_BYTES 5

int sockfd = -1;

void finish(int result) {
  close(sockfd);
#if EMSCRIPTEN
  REPORT_RESULT();
#endif
  exit(result);
}

void iter(void *arg) {
  static int state = 0;
  static char writebuf[] = "01234567890123456789";
  static int writePos = 0;
  static char readbuf[1024];
  static int readPos = 0;
  int selectRes;
  ssize_t transferAmount;
  fd_set sett;  
  
  switch (state) {
    case 0:
      // writing 10 bytes to the server
      
      // since the socket in the read file descriptors has no available data,
      // select should tell us 0 handles are ready
      FD_ZERO(&sett);
      FD_SET(sockfd, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if (selectRes != 0) {
        printf("case 0: read select != 0 (%d)\n", selectRes);
        finish(EXIT_FAILURE);
      }
      
      // the socket in the write file descriptors has to result in either a 0 or 1
      // the connection either is setting up or is established and writing is possible
      FD_ZERO(&sett);
      FD_SET(sockfd, &sett);
      selectRes = select(64, NULL, &sett, NULL, NULL);
      if (selectRes == -1) {
        printf("case 0: write select == -1\n");
        finish(EXIT_FAILURE);
      } else if (selectRes == 0) {
        return;
      }
      
      // send a single byte
      transferAmount = send(sockfd, writebuf+writePos, 1, 0);
      writePos += transferAmount;
   
      // after 10 bytes switch to next state
      if (writePos >= 10) {
        state = 1;
      }
      break;
      
    case 1:
      // wait until we can read one byte to make sure the server
      // has sent the data and then closed the connection
      FD_ZERO(&sett);
      FD_SET(sockfd, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if (selectRes == -1) {
        printf("case 1: read selectRes == -1\n");
        finish(EXIT_FAILURE);
      } else if (selectRes == 0) {
        return;
      }

      // read a single byte
      transferAmount = recv(sockfd, readbuf+readPos, 1, 0);
      readPos += transferAmount;
   
      // if successfully reading 1 byte, switch to next state
      if (readPos >= 1) {
        state = 2;
      }
      break;
    
    case 2:
      // calling select with the socket in the write file descriptors should
      // succeed, but the socket should not set in the set.
      FD_ZERO(&sett);
      FD_SET(sockfd, &sett);
      selectRes = select(64, NULL, &sett, NULL, NULL);
      if (selectRes != 0 || FD_ISSET(sockfd, &sett)) {
        printf("case 2: write selectRes != 0 || FD_ISSET(sockfd, &sett)\n");
        finish(EXIT_FAILURE);
      }

      // calling select with the socket in the read file descriptors 
      // has to succeed because there is still data in the inQueue
      FD_ZERO(&sett);
      FD_SET(sockfd, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if (selectRes != 1) {
        printf("case 2: read selectRes != 1\n");
        finish(EXIT_FAILURE);
      } else if (selectRes == 0) {
        return;
      }
      
      // read a single byte
      transferAmount = recv(sockfd, readbuf+readPos, 1, 0);
      readPos += transferAmount;
      
      // with 10 bytes read the inQueue is empty => switch state
      if (readPos >= 10) {
        state = 3;
      }
      break;
      
    case 3:
      // calling select with the socket in the read file descriptors 
      // should succeed
      FD_ZERO(&sett);
      FD_SET(sockfd, &sett);
      selectRes = select(64, &sett, NULL, NULL, NULL);
      if (selectRes != 1) {
        printf("case 3: read selectRes != 1\n");
        finish(EXIT_FAILURE);
      }

      // but recv should return 0 signaling the remote
      // end has closed the connection.
      transferAmount = recv(sockfd, readbuf, 1, 0);
      if (transferAmount) {
        printf("case 3: read != 0\n");
        finish(EXIT_FAILURE);
      }
      
      // report back success, the 266 is just an arbitrary value without 
      // deeper meaning
      finish(266);
      break;
      
    default:
      printf("Impossible state!\n");
      finish(EXIT_FAILURE);
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
int main() {
  struct sockaddr_in addr;
  int res;

  sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) {
    perror("cannot create socket");
    finish(EXIT_FAILURE);
  }
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SOCKK);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
    perror("inet_pton failed");
    finish(EXIT_FAILURE);
  }

  // This call should succeed (even if the server port is closed)
  res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1 && errno != EINPROGRESS) {
    perror("connect failed");
    finish(EXIT_FAILURE);
  }

#if EMSCRIPTEN
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (1) iter(NULL);
#endif

  return EXIT_SUCCESS;
}

