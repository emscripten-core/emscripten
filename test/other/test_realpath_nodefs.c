#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <emscripten.h>

#define TEST_PATH "/working/TEST_NODEFS.txt"

int main(int argc, char **argv) {
  errno = 0;
  EM_ASM({
    FS.mkdir('/working');
    FS.mount(NODEFS, { root: '.' }, '/working');
  });
  char *t_realpath_buf = realpath(TEST_PATH, NULL);
  if (NULL == t_realpath_buf) {
    perror("Resolve failed");
    return 1;
  } else {
    printf("Resolved: %s\n", t_realpath_buf);
    free(t_realpath_buf);
    return 0;
  }
}
