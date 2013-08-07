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
#if EMSCRIPTEN
#include <emscripten.h>
#endif

#include "websockets_bigdata.h"

#define EXPECTED_BYTES DATA_SIZE

int sockfd;

void finish(int result) {
  close(sockfd);
#if EMSCRIPTEN
  REPORT_RESULT();
#endif
  exit(result);
}

unsigned int get_all_buf(int sock, char* output, unsigned int maxsize) {
  int bytes;
  if (ioctl(sock, FIONREAD, &bytes)) return 0;
  if (bytes == 0) return 0;

  char buffer[EXPECTED_BYTES];
  int n;
  unsigned int offset = 0;
  while((errno = 0, (n = recv(sock, buffer, sizeof(buffer), 0))>0) ||
    errno == EINTR) {
    if (n > 0) {
      if (((unsigned int) n)+offset > maxsize) {
        fprintf(stderr, "too much data!");
        finish(EXIT_FAILURE);
      }
      memcpy(output+offset, buffer, n);
      offset += n;
    }
  }

  if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
    fprintf(stderr, "error in get_all_buf!");
    finish(EXIT_FAILURE);
  }
  return offset;
}

void iter(void *arg) {
  static char out[EXPECTED_BYTES];
  static int pos = 0;
  fd_set fdr;
  int res;

  // make sure that sockfd has finished connecting and is ready to read
  FD_ZERO(&fdr);
  FD_SET(sockfd, &fdr);
  res = select(64, &fdr, NULL, NULL, NULL);
  if (res == -1) {
    perror("select failed");
    finish(EXIT_FAILURE);
    return;
  } else if (!FD_ISSET(sockfd, &fdr)) {
    return;
  }

  // perform read write operations ...
  printf("so far %d, expecting up to %d\n", pos, EXPECTED_BYTES-pos);
  res = get_all_buf(sockfd, out+pos, EXPECTED_BYTES-pos);
  if (res) printf("read! %d\n", res);
  pos += res;
  if (pos >= EXPECTED_BYTES) {
    shutdown(sockfd, SHUT_RDWR);

    close(sockfd);

    char *comp = generateData();
    int result = strcmp(comp, out);
    if (result != 0) {
      for (int i = 0; i < DATA_SIZE; i++) {
        printf("%d:%d\n", comp[i], out[i]);
      }
    }
    finish(result);
  }
}

int main() {
  struct sockaddr_in addr;
  int res;

  printf("hello from main page\n");

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

  res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1 && errno != EINPROGRESS) {
    perror("connect failed");
    finish(EXIT_FAILURE);
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
  while (1) iter(NULL);
#endif

  return EXIT_SUCCESS;
}

