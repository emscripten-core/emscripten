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

int sockfd;
int not_always_data = 0;

void finish(int result) {
  close(sockfd);
#if EMSCRIPTEN
  REPORT_RESULT();
#endif
  exit(result);
}

unsigned int get_all_buf(int sock, char* output, unsigned int maxsize) {
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
    printf("ioctl says 0, FD_ISSET says %ld\n", FD_ISSET(sock, &sett));
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
    if(n > 0) {
      if (((unsigned int) n)+offset > maxsize) {
        fprintf(stderr, "too much data!");
        finish(EXIT_FAILURE);
      }
      memcpy(output+offset, buffer, n);
      offset += n;
    }
  }

  if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
    fprintf(stderr, "error in get_all_buf! %d", errno);
    finish(EXIT_FAILURE);
  }
  return offset;
}

void iter(void *arg) {
  static char out[1024*2];
  static int pos = 0;
  fd_set fdr;
  int res;

  // make sure that sockfd is ready to read
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

void main() {
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
}

