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

int not_always_data = 0;

unsigned int get_all_buf(int sock, char* output, unsigned int maxsize)
{
  // select check for IO
  fd_set sett;
  FD_ZERO(&sett);
  assert(select(64, &sett, NULL, NULL, NULL) == 0); // empty set
  FD_SET(sock, &sett);
  assert(select(0, &sett, NULL, NULL, NULL) == 0); // max FD to check is 0
  assert(FD_ISSET(sock, &sett) == 0);
  FD_SET(sock, &sett);
  int select_says_yes = select(64, &sett, NULL, NULL, NULL);

  // ioctl check for IO
  int bytes;
  if (ioctl(sock, FIONREAD, &bytes) || bytes == 0) {
    not_always_data = 1;
    assert(FD_ISSET(sock, &sett) == 0);
    return 0;
  }

  assert(FD_ISSET(sock, &sett));
  assert(select_says_yes); // ioctl must agree with select

  char buffer[1024];
  int n;
  unsigned int offset = 0;
  while((errno = 0, (n = recv(sock, buffer, sizeof(buffer), 0))>0) ||
    errno == EINTR) {
    if(n>0)
    {
      if (((unsigned int) n)+offset > maxsize) { fprintf(stderr, "too much data!"); exit(EXIT_FAILURE); }
      memcpy(output+offset, buffer, n);
      offset += n;
    }
  }

  if(n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
    fprintf(stderr, "error in get_all_buf! %d", errno);
    exit(EXIT_FAILURE);
  }
  return offset;
}

int done = 0;

void iter(void *arg) {
  /* perform read write operations ... */
  static char out[1024*2];
  static int pos = 0;
  int n = get_all_buf(SocketFD, out+pos, 1024-pos);
  if (n) printf("read! %d\n", n);
  pos += n;
  if (pos >= EXPECTED_BYTES) {
    int i, sum = 0;
    for (i=0; i < pos; i++) {
      printf("%x\n", out[i]);
      sum += out[i];
    }

    shutdown(SocketFD, SHUT_RDWR);

    close(SocketFD);

    done = 1;

    printf("sum: %d\n", sum);

#if EMSCRIPTEN
    //assert(not_always_data == 1);

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

