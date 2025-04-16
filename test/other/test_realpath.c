#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv) {
  char *t_realpath_buf = realpath("/boot/README.txt", NULL);
  if (!t_realpath_buf) {
    perror("Resolve failed");
    return 1;
  }

  printf("Resolved: %s\n", t_realpath_buf);
  free(t_realpath_buf);
  return 0;
}
