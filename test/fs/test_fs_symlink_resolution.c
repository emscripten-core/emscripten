#include <unistd.h>
#include <fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

void makedir(const char *dir) {
  int rtn = mkdir(dir, 0777);
  assert(rtn == 0);
}

void changedir(const char *dir) {
  int rtn = chdir(dir);
  assert(rtn == 0);
}

static void create_file(const char *path) {
  printf("creating: %s\n", path);
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0777);
  assert(fd >= 0);

  close(fd);
}

void setup() {
  makedir("a");
  makedir("b");
  makedir("b/c");
  symlink("../b/c", "a/link");
}


int main() {
  setup();
  create_file("a/link/../x.txt");
  struct stat statBuf;
  assert(stat("a/link/../x.txt", &statBuf) == 0);
  assert(stat("b/x.txt", &statBuf) == 0);
  makedir("a/link/../d");
  assert(stat("a/link/../d", &statBuf) == 0);
  assert(stat("b/d", &statBuf) == 0);

  assert(truncate("a/link/../x.txt", 0) == 0);
  assert(chmod("a/link/../x.txt", 0777) == 0);
  printf("success\n");
}
