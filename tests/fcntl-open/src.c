#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main() {
  struct stat s;
  int modes[] = {O_RDONLY, O_WRONLY, O_RDWR};
  char nonexistent_name[] = "/noexist-##";

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 16; j++) {
      int flags = modes[i];
      if (j & 0x1) flags |= O_CREAT;
      if (j & 0x2) flags |= O_EXCL;
      if (j & 0x4) flags |= O_TRUNC;
      if (j & 0x8) flags |= O_APPEND;

      printf("EXISTING FILE %d,%d\n", i, j);
      printf("success: %d\n", open("/test-file", flags, 0777) != -1);
      printf("errno: %d\n", errno);
      stat("/test-file", &s);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;

      printf("EXISTING FOLDER %d,%d\n", i, j);
      printf("success: %d\n", open("/test-folder", flags, 0777) != -1);
      printf("errno: %d\n", errno);
      stat("/test-folder", &s);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;

      nonexistent_name[9] = 'a' + i;
      nonexistent_name[10] = 'a' + j;
      printf("NON-EXISTING %d,%d\n", i, j);
      printf("success: %d\n", open(nonexistent_name, flags, 0777) != -1);
      printf("errno: %d\n", errno);
      stat(nonexistent_name, &s);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;
    }
  }

  printf("CREAT\n");
  printf("success: %d\n", creat("/creat-me", 0777) != -1);
  printf("errno: %d\n", errno);

  return 0;
}
