#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include "stdio.h"

#ifdef __EMSCRIPTEN__
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


int main() {
    makedir("working");
#if defined(__EMSCRIPTEN__) && defined(NODEFS)
    EM_ASM(FS.mount(NODEFS, { root: '.' }, 'working'));
#endif
    changedir("working");
    int fd = open("file.txt", O_RDWR | O_CREAT, 0666);
    unlink("file.txt");
    int res;
    struct stat buf;
    res = fstat(fd, &buf);
    assert(res == 0);
    assert(buf.st_atime > 1000000000);
    res = fchmod(fd, 0777);
    assert(res == 0);
    res = ftruncate(fd, 10);
    assert(res == 0);
    printf("success\n");
}
