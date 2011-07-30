#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main() {
  struct stat s;
  int f = open("/towrite", O_WRONLY);
  int f2 = open("/toread", O_RDONLY);
  printf("f2: %d\n", f2);

  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(10): %d\n", ftruncate(f, 10));
  printf("errno: %d\n", errno);
  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(4): %d\n", ftruncate(f, 4));
  printf("errno: %d\n", errno);
  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(-1): %d\n", ftruncate(f, -1));
  printf("errno: %d\n", errno);
  fstat(f, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(2): %d\n", truncate("/towrite", 2));
  printf("errno: %d\n", errno);
  stat("/towrite", &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(readonly, 2): %d\n", truncate("/toread", 2));
  printf("errno: %d\n", errno);
  stat("/toread", &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(readonly, 4): %d\n", ftruncate(f2, 4));
  printf("errno: %d\n", errno);
  fstat(f2, &s);
  printf("st_size: %d\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;

  return 0;
}
