#include <sys/types.h>
#include <ifaddrs.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main() {
   struct ifaddrs* ifaddrs = NULL;
   int rtn = getifaddrs(&ifaddrs);
   printf("getifaddrs => %d (%s)\n", rtn, strerror(errno));
   return 0;
}
