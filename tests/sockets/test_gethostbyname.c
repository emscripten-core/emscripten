#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if EMSCRIPTEN
#include <emscripten.h>
#endif

int main() {
  char str[INET_ADDRSTRLEN];
  struct in_addr addr;
  const char *res;
  int err;
  
  // resolve the hostname ot an actual address
  struct hostent *host = gethostbyname("slashdot.org");
  assert(host->h_addrtype == AF_INET);
  assert(host->h_length == sizeof(uint32_t));

  // convert the raw address to a string
  char **raw_addr_list = host->h_addr_list;
  int *raw_addr = (int*)*raw_addr_list;
  res = inet_ntop(host->h_addrtype, raw_addr, str, INET_ADDRSTRLEN);
  assert(res);

  // convert the string to an in_addr structure
  err = inet_pton(AF_INET, str, &addr);
  assert(err == 1);

  // do a reverse lookup on the ip address
  struct hostent *host1 = gethostbyaddr(&addr, sizeof(addr), host->h_addrtype);
  assert(strstr(host1->h_name, "slashdot.org"));

  puts("success");

  return EXIT_SUCCESS;
}

