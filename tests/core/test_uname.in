#include <stdio.h>
#include <sys/utsname.h>

int main() {
  struct utsname u;
  printf("ret: %d\n", uname(&u));
  printf("sysname: %s\n", u.sysname);
  printf("nodename: %s\n", u.nodename);
  printf("release: %s\n", u.release);
  printf("version: %s\n", u.version);
  printf("machine: %s\n", u.machine);
  printf("invalid: %d\n", uname(0));
  return 0;
}
