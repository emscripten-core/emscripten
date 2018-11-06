/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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

int result = 0;
int sock;
char buf[BUFLEN];
char expected[] = "emscripten";
struct sockaddr_in si_host,
                   si_peer;
struct iovec iov[1];
struct msghdr hdr;
int done = 0;

void iter() {
  int n;
  n = recvmsg(sock, &hdr, 0);

  if(0 < n) {
    done = 1;
    fprintf(stderr, "received %d bytes: %s", n, (char*)hdr.msg_iov[0].iov_base);

    shutdown(sock, SHUT_RDWR);
    close(sock);

#ifdef __EMSCRIPTEN__
    if(strlen((char*)hdr.msg_iov[0].iov_base) == strlen(expected) &&
       0 == strncmp((char*)hdr.msg_iov[0].iov_base, expected, strlen(expected))) {
      result = 1;
    }
    REPORT_RESULT(result);
    exit(EXIT_SUCCESS);
    emscripten_cancel_main_loop();
#endif
  } else if(EWOULDBLOCK != errno) {
    perror("recvmsg failed");
    exit(EXIT_FAILURE);
    emscripten_cancel_main_loop();
  }
}

int main(void)
{
  memset(&si_host, 0, sizeof(struct sockaddr_in));
  memset(&si_peer, 0, sizeof(struct sockaddr_in));

  si_host.sin_family = AF_INET;
  si_host.sin_port = htons(8991);
  si_host.sin_addr.s_addr = htonl(INADDR_ANY);

  if(-1 == (sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
    perror("cannot create host socket");
    exit(EXIT_FAILURE);
  }

  if(-1 == bind(sock, (struct sockaddr*)&si_host, sizeof(struct sockaddr))) {
    perror("cannot bind host socket");
    exit(EXIT_FAILURE);
  }

  iov[0].iov_base = buf;
  iov[0].iov_len = sizeof(buf);

  memset (&hdr, 0, sizeof (struct msghdr));

  hdr.msg_name = &si_peer;
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
