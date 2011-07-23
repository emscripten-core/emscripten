#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main() {
  char host[256] = "--------------------------";

  printf("gethostid: %d\n", gethostid());
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("gethostname/2 ret: %d\n", gethostname(host, 2));
  printf("gethostname/2: %s\n", host);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("gethostname/256 ret: %d\n", gethostname(host, 256));
  printf("gethostname/256: %s\n", host);
  printf("errno: %d\n\n", errno);
  errno = 0;

  char login[256] = "--------------------------";

  printf("login: %s\n", getlogin());
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("login_r/2 ret: %d\n", getlogin_r(login, 2));
  printf("login_r/2: %s\n", login);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("login_r/256 ret: %d\n", getlogin_r(login, 256));
  printf("login_r/256: %s\n", login);
  printf("errno: %d\n", errno);
  errno = 0;

  return 0;
}
