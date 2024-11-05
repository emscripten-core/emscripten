#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int testrealpath(const char* path) {
  errno = 0;
  char *t_realpath_buf = realpath(path, NULL);
  if (NULL == t_realpath_buf) {
    printf("Resolve failed: \"%s\"\n",path);fflush(stdout);
    return 1;
  } else {
    printf("Resolved: \"%s\" => \"%s\"\n", path, t_realpath_buf);fflush(stdout);
    free(t_realpath_buf);
    return 0;
  }
}

int main(int argc, char **argv) {
  // files:
  testrealpath("testfile.txt");
  testrealpath("Folder/testfile.txt");
  testrealpath("testnonexistentfile.txt");
  // folders
  testrealpath("Folder");
  testrealpath("/Folder");
  testrealpath("./");
  testrealpath("");
  testrealpath("/");
  return 0;
}
