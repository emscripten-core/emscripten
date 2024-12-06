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

static void create_file(const char *path, const char *buffer) {
  printf("creating: %s\n", path);
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
  printf("error: %s\n", strerror(errno));
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
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
    create_file("a", "abc");
    create_file("b", "xyz");
    assert(rename("a", "b") == 0);
    assert(unlink("b") == 0);
    create_file("b", "xyz");
    printf("success\n");
}
