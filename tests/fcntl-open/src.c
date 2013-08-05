#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

char nonexistent_name[] = "noexist-##";

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  create_file("test-file", "abcdef", 0777);
  mkdir("test-folder", 0777);
}

void cleanup() {
  unlink("test-file");
  rmdir("test-folder");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 16; j++) {
      nonexistent_name[8] = 'a' + i;
      nonexistent_name[9] = 'a' + j;
      unlink(nonexistent_name);
    }
  }
  unlink("creat-me");
}

void test() {
  struct stat s;
  int modes[] = {O_RDONLY, O_WRONLY, O_RDWR};

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 16; j++) {
      int flags = modes[i];
      if (j & 0x1) flags |= O_CREAT;
      if (j & 0x2) flags |= O_EXCL;
      if (j & 0x4) flags |= O_TRUNC;
      if (j & 0x8) flags |= O_APPEND;

      printf("EXISTING FILE %d,%d\n", i, j);
      printf("success: %d\n", open("test-file", flags, 0777) != -1);
      printf("errno: %d\n", errno);
      stat("test-file", &s);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;

      printf("EXISTING FOLDER %d,%d\n", i, j);
      printf("success: %d\n", open("test-folder", flags, 0777) != -1);
      printf("errno: %d\n", errno);
      stat("test-folder", &s);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;

      nonexistent_name[8] = 'a' + i;
      nonexistent_name[9] = 'a' + j;
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
  printf("success: %d\n", creat("creat-me", 0777) != -1);
  printf("errno: %d\n", errno);
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
