#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <string.h>


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

  int res = open("a", O_CREAT, 0);
  printf("error: %s\n", strerror(errno));
  assert(res >= 0);
  struct stat st;
  assert(stat("a", &st) == 0);
  assert((st.st_mode & 0777) == 0);
  printf("success\n");
}
