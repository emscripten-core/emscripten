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
  open("./does-not-exist/", O_CREAT);
  assert(errno == EISDIR);
  printf("success\n");
}
