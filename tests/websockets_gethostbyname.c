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

#define EXPECTED_BYTES 5

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

  char buffer[1024];
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
    perror("error in get_all_buf!");
    finish(EXIT_FAILURE);
  }
  return offset;
}

void iter() {
  static char out[1024*2];
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
  } else if (!FD_ISSET(sockfd, &fdr)) {
    return;
  }

  // perform read write operations ...
  int n = get_all_buf(sockfd, out+pos, 1024-pos);
  if (n) printf("read! %d\n", n);
  pos += n;
  if (pos >= EXPECTED_BYTES) {
    int i, sum = 0;
    for (i=0; i < pos; i++) {
      printf("%x\n", out[i]);
      sum += out[i];
    }

    shutdown(sockfd, SHUT_RDWR);

    close(sockfd);

    printf("sum: %d\n", sum);

    finish(sum);
  }
}

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
  struct hostent *host0 = gethostbyname("test.com"); // increment hostname counter to check for possible but at 0,0 not differentiating low/high
  struct hostent *host = gethostbyname("localhost");
  char **raw_addr_list = host->h_addr_list;
  int *raw_addr = (int*)*raw_addr_list;
  printf("raw addr: %d\n", *raw_addr);
  char name[INET_ADDRSTRLEN];
  if (!inet_ntop(AF_INET, raw_addr, name, sizeof(name))) {
    printf("could not figure out name\n");
    finish(EXIT_FAILURE);
  }
  printf("localhost has 'ip' of %s\n", name);

  if (inet_pton(AF_INET, name, &addr.sin_addr) != 1) {
    perror("inet_pton failed");
    finish(EXIT_FAILURE);
  }

  res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  if (res == -1 && errno != EINPROGRESS) {
    perror("connect failed");
    finish(EXIT_FAILURE);
  }

#if EMSCRIPTEN
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (1) iter();
#endif

  return EXIT_SUCCESS;
}

