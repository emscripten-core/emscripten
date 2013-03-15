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

#include "websockets_bigdata.h"

#define EXPECTED_BYTES DATA_SIZE

int SocketFD;

unsigned int get_all_buf(int sock, char* output, unsigned int maxsize)
{
  int bytes;
  if (ioctl(sock, FIONREAD, &bytes)) return 0;
  if (bytes == 0) return 0;

  char buffer[EXPECTED_BYTES];
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
    fprintf(stderr, "error in get_all_buf!");
    exit(EXIT_FAILURE);
  }
  return offset;
}

int done = 0;

void iter(void *arg) {
  /* perform read write operations ... */
  static char out[EXPECTED_BYTES];
  static int pos = 0;
  printf("so far %d, expecting up to %d\n", pos, EXPECTED_BYTES-pos);
  int n = get_all_buf(SocketFD, out+pos, EXPECTED_BYTES-pos);
  if (n) printf("read! %d\n", n);
  pos += n;
  if (pos >= EXPECTED_BYTES) {
    shutdown(SocketFD, SHUT_RDWR);

    close(SocketFD);

    done = 1;

    emscripten_cancel_main_loop();

#if EMSCRIPTEN
    char *comp = generateData();
    int result = strcmp(comp, out);
    if (result != 0) {
      for (int i = 0; i < DATA_SIZE; i++) {
        printf("%d:%d\n", comp[i], out[i]);
      }
    }
    REPORT_RESULT();
#endif
  }
}

int main(void)
{
  printf("hello from main page\n");

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
    3993
#else
    3992
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
  emscripten_run_script("console.log('adding iframe');"
                        "var iframe = document.createElement('iframe');"
                        "iframe.src = 'side.html';"
                        "iframe.width = '100%';"
                        "iframe.width = '40%';"
                        "document.body.appendChild(iframe);"
                        "console.log('added.');");
  emscripten_set_main_loop(iter, 3, 0);
#else
  while (!done) iter(NULL);
#endif

  return EXIT_SUCCESS;
}

