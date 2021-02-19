#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
  DIR *d = opendir("/proc/self/fd");
  if (!d) {
    printf("error opening proc/self");
    return 1;
  }

  struct dirent *entry;
  while ((entry = readdir(d))) {
    printf("entry: %s\n", entry->d_name);
    printf("unexpected success reading directory!");
    return 1;
  }
  closedir(d);

  char buf[1024];
  ssize_t s = readlink("/proc/self/fd/0", buf, 1024);
  assert(s > 0);
  // readlink does *not* append a null, so print out just the correct number of
  // characters.
  printf("fd0: %.8s\n", buf);

  printf("done\n");
  return 0;
}

