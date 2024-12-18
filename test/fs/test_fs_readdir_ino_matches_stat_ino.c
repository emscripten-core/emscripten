#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

void setup() {
  int res = open("b", O_CREAT, 0777);
  assert(res >= 0);
  assert(close(res) == 0);
  assert(symlink("b", "a") == 0);
}

int main() {
  setup();
  int dirfd = open(".", O_RDONLY);
  assert(dirfd > 0);
  DIR *dirp = fdopendir(dirfd);
  assert(dirp != NULL);
  struct stat sta, stb;
  assert(lstat("a", &sta) == 0);
  assert(lstat("b", &stb) == 0);
  // Remove execute permission from directory. This prevents us from stat'ing
  // files in the directory in the implementation of readdir which we tried to
  // use to fix this.
  assert(chmod(".", 0675) == 0);
  int a_ino = -1;
  int b_ino = -1;
  struct dirent *ep;
  while ((ep = readdir(dirp))) {
    if (strcmp(ep->d_name, "a") == 0) {
      a_ino = ep->d_ino;
    }
    if (strcmp(ep->d_name, "b") == 0) {
      b_ino = ep->d_ino;
    }
  }
  assert(errno == 0);
  assert(a_ino >= 0);
  assert(b_ino >= 0);
  printf("readdir a_ino: %d, b_ino: %d\n", a_ino, b_ino);
  printf("stat    a_ino: %llu, b_ino: %llu\n", sta.st_ino, stb.st_ino);
  assert(a_ino == sta.st_ino);
  assert(b_ino == stb.st_ino);
  printf("success\n");
  return 0;
}
