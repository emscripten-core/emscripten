/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


void do_ip_check(char* name) {
  char str[INET_ADDRSTRLEN];
  struct hostent *host = NULL;
  struct hostent hostData;
  struct in_addr addr;
  const char *res;
  char buffer[2048];
  int err;

  // gethostbyname_r calls the same stuff as gethostbyname, so we'll test the
  // more complicated one.
  // resolve the hostname to an actual address
  gethostbyname_r(name, &hostData, buffer, sizeof(buffer), &host, &err);
  assert(host->h_addrtype == AF_INET);
  assert(host->h_length == sizeof(uint32_t));

  // convert the raw address to a string
  char **raw_addr_list = host->h_addr_list;
  int *raw_addr = (int*)*raw_addr_list;
  res = inet_ntop(host->h_addrtype, raw_addr, str, INET_ADDRSTRLEN);
  printf("gethostbyname -> %s\n", res);
  assert(res);

  // convert the string to an in_addr structure
  err = inet_pton(AF_INET, str, &addr);
  assert(err == 1);

  // do a reverse lookup on the ip address
  struct hostent *host1 = gethostbyaddr(&addr, sizeof(addr), host->h_addrtype);
  printf("gethostbyaddr -> %s\n", host1->h_name);
  assert(strstr(host1->h_name, name));
}

int main() {
  printf("slashdot.org\n");
  do_ip_check("slashdot.org");
  
  printf("localhost\n");
  do_ip_check("localhost");
  
  puts("success");
  return EXIT_SUCCESS;
}

