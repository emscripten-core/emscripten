#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

int main(void) {
  for (int fd = 0; fd < 3; fd++) {
    struct stat st;
    int ret = fstat(fd, &st);
    assert(ret == 0);
    assert(st.st_mode & S_IFMT);
  }
  printf("done\n");
  return 0;
}
