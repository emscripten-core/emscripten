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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define EXPECTED_BYTES 5
#define BUFLEN 16
#define HOST_ADDR "10.0.0.1"

int result = 0;
int sock;
char buf[16] = "emscripten";
struct sockaddr_in si_host;
struct iovec iov[1];
struct msghdr hdr;
int done = 0;

void iter() {
  int n;
  n = sendmsg(sock, &hdr, 0);

  if(0 < n) {
    done = 1;
    fprintf(stderr, "sent %d bytes: %s", n, (char*)hdr.msg_iov[0].iov_base);

    shutdown(sock, SHUT_RDWR);
    close(sock);

    exit(EXIT_SUCCESS);
    emscripten_cancel_main_loop();
  } else if(EWOULDBLOCK != errno) {
    perror("sendmsg failed");
    exit(EXIT_FAILURE);
    emscripten_cancel_main_loop();
  }
}

int main(void)
{
  memset(&si_host, 0, sizeof(struct sockaddr_in));

  si_host.sin_family = AF_INET;
  si_host.sin_port = htons(8991);
  if(0 == inet_pton(AF_INET, HOST_ADDR, &si_host.sin_addr)) {
    perror("inet_aton failed");
    exit(EXIT_FAILURE);
  }

  if(-1 == (sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  iov[0].iov_base = buf;
  iov[0].iov_len = sizeof(buf);

  memset (&hdr, 0, sizeof (struct msghdr));

  hdr.msg_name = &si_host;
  hdr.msg_namelen = sizeof(struct sockaddr_in);
  hdr.msg_iov = iov;
  hdr.msg_iovlen = 1;

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (!done) iter();
#endif

  return EXIT_SUCCESS;
}
