#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
  char buf[10] = {0};

  int cnt = 0;
  while (!feof(stdin)) {
    int cnt = fread(buf, 1, 3, stdin);
    printf("read %d bytes: '%s'\n", cnt, buf);
    fflush(stdin);
    memset(buf, 0, sizeof(buf));
  }

  printf("done\n");
  return 0;
}
