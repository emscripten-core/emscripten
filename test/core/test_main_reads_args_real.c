#include <stdio.h>
#include <string.h>

int real_main(int argc, char** argv) {
  printf("argc: %d\n", argc);
  const char* arg0 = strrchr(argv[0], '/');
  if (!arg0)
    arg0 = strrchr(argv[0], '\\');
  printf("argv[0]: %s\n", arg0 + 1);
  return 0;
}
