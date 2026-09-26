#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

int main(void) {
  for (int fd = 0; fd < 3; fd++) {
    struct stat st;
    int ret = fstat(fd, &st);
    assert(ret == 0);
    // A standard stream is a tty, or a pipe or regular file when redirected
    // (as under NODERAWFS, where it reflects the real inherited descriptor).
    assert(S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISREG(st.st_mode));
  }
  printf("done\n");
  return 0;
}
