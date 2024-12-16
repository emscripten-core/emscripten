#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

void makedir(const char *dir) {
  int rtn = mkdir(dir, 0777);
  assert(rtn == 0);
}

void changedir(const char *dir) {
  int rtn = chdir(dir);
  assert(rtn == 0);
}

void setup() {
#if defined(__EMSCRIPTEN__) && defined(NODEFS)
  makedir("working");
  EM_ASM(FS.mount(NODEFS, { root: '.' }, 'working'));
  changedir("working");
#endif
}

int main() {
  setup();
  mkdir("test", 0777);
  mkdir("test/a", 0777);
  mkdir("test/a/..", 0777);
  printf("error: %s\n", strerror(errno));
  assert(errno == EEXIST);
  mkdir("test/a/.", 0777);
  printf("error: %s\n", strerror(errno));
  assert(errno == EEXIST);
  printf("success\n");
}

