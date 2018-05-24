#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv) {
  int retval = open("NonExistingFile", O_RDONLY);
  if (retval == -1) perror("Cannot open NonExistingFile");
  return 0;
}
