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

#define EXPECTED_BYTES 28

int ListenFD, SocketFD;

unsigned int get_all_buf(int sock, char* output, unsigned int maxsize)
{
  int bytes;
  if (ioctl(sock, FIONREAD, &bytes)) return 0;
  if (bytes == 0) return 0;

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
    fprintf(stderr, "error in get_all_buf!");
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
    int result = sum;
    REPORT_RESULT();
    emscripten_cancel_main_loop();
#endif
  }
}

int main(void)
{
  struct sockaddr_in stSockAddr;
  int Res;
  ListenFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == ListenFD)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(stSockAddr));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(
#if EMSCRIPTEN
    6993
#else
    6995
#endif
  );
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res) {
    perror("error: first parameter is not a valid address family");
    close(ListenFD);
    exit(EXIT_FAILURE);
  } else if (0 == Res) {
    perror("char string (second parameter does not contain valid ipaddress)");
    close(ListenFD);
    exit(EXIT_FAILURE);
  }

  printf("bind..\n");

  if (-1 == bind(ListenFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
    perror("bind failed");
    close(ListenFD);
    exit(EXIT_FAILURE);
  }

  printf("listen..\n");

  if (-1 == listen(ListenFD, 50)) {
    perror("listen failed");
    close(ListenFD);
    exit(EXIT_FAILURE);
  }  

  printf("accept..\n");

  struct sockaddr_in stSockAddr2;
  socklen_t temp;

  if (-1 == (SocketFD = accept(ListenFD, (struct sockaddr *)&stSockAddr2, &temp))) {
    perror("accept failed");
    close(ListenFD);
    exit(EXIT_FAILURE);
  }  

#if EMSCRIPTEN
  emscripten_run_script("console.log('adding iframe');"
                        "var iframe = document.createElement('iframe');"
                        "iframe.src = 'side.html';"
                        "document.body.appendChild(iframe);"
                        "console.log('added.');");
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (!done) iter(NULL);
#endif

  return EXIT_SUCCESS;
}

